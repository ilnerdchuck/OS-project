// s32k_tpm_simple.c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "hw/registerfields.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include <string.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include "hw/ssi/S32K_TPM.h"


/* helper: set status flags */
static void tpm_set_status(S32KTPMState *s, uint32_t set, uint32_t clear) {
    s->status_reg = (s->status_reg & ~clear) | set;
}

/* helper: write data_out and mark ready */
static void tpm_write_response(S32KTPMState *s, const uint8_t *buf, size_t len) {
    if (len > DATA_OUT_SIZE) len = DATA_OUT_SIZE;
    memcpy(s->data_out, buf, len);
    /* zero remainder for clean reads */
    if (len < DATA_OUT_SIZE) {
        memset(s->data_out + len, 0, DATA_OUT_SIZE - len);
    }
    tpm_set_status(s, STATUS_READY, STATUS_BUSY | STATUS_ERROR);
}

/* Generate random key into slot */
static void tpm_generate_key(S32KTPMState *s, uint32_t slot) {
    if (slot >= KEY_SLOT_COUNT) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: invalid key slot %u\n", slot);
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    if (RAND_bytes(s->key_store[slot], KEY_BYTES) != 1) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: RNG failed\n");
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    s->key_valid[slot] = true;

    uint8_t resp[4];
    resp[0] = (uint8_t)slot;
    tpm_write_response(s, resp, 1);
}

/* Return key blob (insecure: returns raw symmetric key) */
static void tpm_get_key(S32KTPMState *s, uint32_t slot) {
    if (slot >= KEY_SLOT_COUNT || !s->key_valid[slot]) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: get_key invalid slot %u\n", slot);
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    tpm_write_response(s, s->key_store[slot], KEY_BYTES);
}

/* Sign (HMAC-SHA256) using slot key */
static void tpm_sign(S32KTPMState *s, uint32_t slot, const uint8_t *msg, size_t msg_len) {
    if (slot >= KEY_SLOT_COUNT || !s->key_valid[slot]) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: sign invalid slot %u\n", slot);
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    unsigned int sig_len = SHA256_DIGEST_LENGTH;
    uint8_t sig[SHA256_DIGEST_LENGTH];

    HMAC(EVP_sha256(),
         s->key_store[slot], KEY_BYTES,
         msg, msg_len, sig, &sig_len);

    tpm_write_response(s, sig, sig_len);
}

/* PCR extend: pcr = SHA256(pcr || newhash) where newhash = SHA256(data) */
static void tpm_pcr_extend(S32KTPMState *s, uint32_t pcr_index,
                           const uint8_t *data, size_t data_len) {
    if (pcr_index >= PCR_COUNT) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: pcr_extend invalid index %u\n", pcr_index);
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }

    uint8_t newhash[SHA256_DIGEST_LENGTH];
    SHA256(data, data_len, newhash);

    /* concat current PCR and newhash and SHA256 */
    uint8_t tmp[SHA256_DIGEST_LENGTH * 2];
    memcpy(tmp, s->pcr[pcr_index], SHA256_DIGEST_LENGTH);
    memcpy(tmp + SHA256_DIGEST_LENGTH, newhash, SHA256_DIGEST_LENGTH);

    uint8_t result[SHA256_DIGEST_LENGTH];
    SHA256(tmp, sizeof(tmp), result);

    memcpy(s->pcr[pcr_index], result, SHA256_DIGEST_LENGTH);

    tpm_write_response(s, result, SHA256_DIGEST_LENGTH);
}

/* PCR read */
static void tpm_pcr_read(S32KTPMState *s, uint32_t pcr_index) {
    if (pcr_index >= PCR_COUNT) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: pcr_read invalid index %u\n", pcr_index);
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    tpm_write_response(s, s->pcr[pcr_index], SHA256_DIGEST_LENGTH);
}

/* NV write/read (simple fixed-size slots) */
static void tpm_nv_write(S32KTPMState *s, uint32_t nv_index,
                         const uint8_t *data, size_t len) {
    if (nv_index >= NV_SLOTS || len > NV_SLOT_SIZE) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: nv_write invalid index/len\n");
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    memcpy(s->nv_slots[nv_index], data, len);
    s->nv_len[nv_index] = len;
    tpm_write_response(s, (const uint8_t *)"OK", 2);
}

static void tpm_nv_read(S32KTPMState *s, uint32_t nv_index) {
    if (nv_index >= NV_SLOTS) {
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: nv_read invalid index\n");
        tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
        return;
    }
    tpm_write_response(s, s->nv_slots[nv_index], s->nv_len[nv_index]);
}

/* Main dispatcher: read DATA_IN and run the command */
static void tpm_handle_cmd(S32KTPMState *s, uint32_t cmd) {
    tpm_set_status(s, STATUS_BUSY, STATUS_READY | STATUS_ERROR);

    uint8_t *in = s->data_in;
    switch (cmd) {
        case CMD_GENERATE_KEY:
            /* in[0] = slot */
            tpm_generate_key(s, (uint32_t)in[0]);
            break;
        case CMD_GET_KEY:
            tpm_get_key(s, (uint32_t)in[0]);
            break;
        case CMD_SIGN: {
            uint32_t slot = in[0];
            size_t msg_len = DATA_IN_SIZE - 1;
            /* find actual length: assume guest sets length in control lower 16 bits */
            uint32_t ctl = s->control_reg;
            if (ctl & 0xFFFF) msg_len = ctl & 0xFFFF;
            tpm_sign(s, slot, in + 1, msg_len);
            break;
        }
        case CMD_PCR_EXTEND: {
            uint32_t pcr = in[0];
            size_t data_len = DATA_IN_SIZE - 1;
            uint32_t ctl = s->control_reg;
            if (ctl & 0xFFFF) data_len = ctl & 0xFFFF;
            tpm_pcr_extend(s, pcr, in + 1, data_len);
            break;
        }
        case CMD_PCR_READ:
            tpm_pcr_read(s, (uint32_t)in[0]);
            break;
        case CMD_NV_WRITE: {
            uint32_t nv = in[0];
            uint32_t ctl = s->control_reg;
            size_t data_len = (ctl & 0xFFFF) ? (ctl & 0xFFFF) : (DATA_IN_SIZE - 1);
            tpm_nv_write(s, nv, in + 1, data_len);
            break;
        }
        case CMD_NV_READ:
            tpm_nv_read(s, (uint32_t)in[0]);
            break;
        default:
            qemu_log_mask(LOG_GUEST_ERROR, "TPM: unknown cmd %u\n", cmd);
            tpm_set_status(s, STATUS_ERROR, STATUS_BUSY);
            break;
    }
}

/* Read handler for MMIO */
static uint64_t s32k_tpm_read(void *opaque, hwaddr offset, unsigned size) {
    S32KTPMState *s = (S32KTPMState *)opaque;

    switch (offset) {
    case OFF_CONTROL:
        return s->control_reg;
    case OFF_STATUS:
        return s->status_reg;
    case OFF_CMD:
        /* CMD register read returns 0 */
        return 0;
    case OFF_DATA_IN:
        /* read first 64-bits safely */
        return (uint64_t) *(uint64_t *)&s->data_in[0];
    case OFF_DATA_OUT:
        /* return first 64-bits of data_out */
        return (uint64_t) *(uint64_t *)&s->data_out[0];
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "TPM: Invalid read offset 0x%" HWADDR_PRIx "\n", offset);
        return 0;
    }
}

/* Write handler for MMIO */
static void s32k_tpm_write(void *opaque, hwaddr offset, uint64_t value, unsigned size) {
    S32KTPMState *s = (S32KTPMState *)opaque;

    switch (offset) {
    case OFF_CONTROL:
        s->control_reg = (uint32_t)value;
        break;
    case OFF_CMD: {
        uint32_t cmd = (uint32_t)value;
        /* cmd trigger: run using data_in buffer */
        /* Note: guest must write DATA_IN region before writing CMD */
        tpm_handle_cmd(s, cmd);
        break;
    }
    case OFF_DATA_IN: {
        /* writes to the first 8 bytes of DATA_IN; guest should also write the whole region via burst */
        memcpy(&s->data_in[0], &value, (size > DATA_IN_SIZE) ? DATA_IN_SIZE : size);
        break;
    }
    case OFF_DATA_OUT:
        /* writing to DATA_OUT is ignored */
        break;
    default:
        /* for simplicity, write into data_in ranges if guest writes beyond first qword */
        if (offset >= OFF_DATA_IN && offset < OFF_DATA_IN + DATA_IN_SIZE) {
            hwaddr idx = offset - OFF_DATA_IN;
            if (idx + size <= DATA_IN_SIZE) {
                memcpy(&s->data_in[idx], &value, size);
            } else {
                qemu_log_mask(LOG_GUEST_ERROR, "TPM: data_in write overflow\n");
            }
        } else if (offset >= OFF_DATA_OUT && offset < OFF_DATA_OUT + DATA_OUT_SIZE) {
            /* ignore */
        } else {
            qemu_log_mask(LOG_GUEST_ERROR, "TPM: Invalid write offset 0x%" HWADDR_PRIx "\n", offset);
        }
        break;
    }
}

/* MemoryRegionOps */
static const MemoryRegionOps s32k_tpm_ops = {
    .read = s32k_tpm_read,
    .write = s32k_tpm_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
};

static void s32k_tpm_reset(DeviceState *dev) {
    S32KTPMState *s = S32K_TPM(dev);
    s->control_reg = 0;
    s->status_reg = 0;
    memset(s->data_in, 0, sizeof(s->data_in));
    memset(s->data_out, 0, sizeof(s->data_out));
    for (int i = 0; i < KEY_SLOT_COUNT; ++i) {
        s->key_valid[i] = false;
        memset(s->key_store[i], 0, KEY_BYTES);
    }
    for (int i = 0; i < PCR_COUNT; ++i) {
        memset(s->pcr[i], 0, PCR_SIZE);
    }
    for (int i = 0; i < NV_SLOTS; ++i) {
        s->nv_len[i] = 0;
        memset(s->nv_slots[i], 0, NV_SLOT_SIZE);
    }
}

static void s32k_tpm_init(Object *obj) {
    S32KTPMState *s = S32K_TPM(obj);
    memory_region_init_io(&s->iomem, obj, &s32k_tpm_ops, s, "s32k_tpm", MMIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    /* Optionally initialize a built-in attestation key in slot 0 */
    if (RAND_bytes(s->key_store[0], KEY_BYTES) == 1) {
        s->key_valid[0] = true;
    }
    /* mark not busy */
    s->status_reg = 0;
}

static const TypeInfo s32k_tpm_info = {
    .name = TYPE_S32K_TPM,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32KTPMState),
    .instance_init = s32k_tpm_init,
    .class_init = NULL,
};

static void s32k_tpm_register_types(void) {
    type_register_static(&s32k_tpm_info);
}

type_init(s32k_tpm_register_types);
