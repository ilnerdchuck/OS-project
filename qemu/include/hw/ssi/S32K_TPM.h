#ifndef HW_MISC_S32K_TPM_H
#define HW_MISC_S32K_TPM_H

#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qom/object.h"
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "migration/vmstate.h"
#include <string.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#define TYPE_S32K_TPM "S32K_TPM"
OBJECT_DECLARE_SIMPLE_TYPE(S32KTPMState, S32K_TPM)

#define MMIO_SIZE 0x310

/* Offsets */
#define OFF_CONTROL    0x00
#define OFF_STATUS     0x04
#define OFF_CMD        0x08
#define OFF_DATA_IN    0x10
#define OFF_DATA_OUT   0x110

/* STATUS bits */
#define STATUS_BUSY    (1 << 0)
#define STATUS_READY   (1 << 1)
#define STATUS_ERROR   (1 << 2)

/* Commands */
#define CMD_GENERATE_KEY  1
#define CMD_GET_KEY       2
#define CMD_SIGN          3
#define CMD_PCR_EXTEND    4
#define CMD_PCR_READ      5
#define CMD_NV_WRITE      6
#define CMD_NV_READ       7
#define CMD_VERIFY_FW     8   /* NEW: verify firmware hash + signature */

#define CMD_VERIFY_BOOT_HASH 0x10  // New command for secure boot
#define BOOT_HASH_SIZE SHA256_DIGEST_LENGTH

/* Sizes */
#define KEY_SLOT_COUNT 4
#define KEY_BYTES 32           /* 256-bit symmetric keys (HMAC) for prototype */
#define PCR_COUNT 24
#define PCR_SIZE 32
#define NV_SLOTS 4
#define NV_SLOT_SIZE 64
#define DATA_IN_SIZE 256
#define DATA_OUT_SIZE 512

typedef struct S32KTPMState {
    /* Sysbus device parent is implied by registration */
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    uint32_t control_reg;
    uint32_t status_reg;

    uint8_t data_in[DATA_IN_SIZE];
    uint8_t data_out[DATA_OUT_SIZE];

    /* key storage */
    uint8_t key_store[KEY_SLOT_COUNT][KEY_BYTES];
    bool    key_valid[KEY_SLOT_COUNT];

    /* PCRs */
    uint8_t pcr[PCR_COUNT][PCR_SIZE];

    /* NV storage */
    uint8_t nv_slots[NV_SLOTS][NV_SLOT_SIZE];
    uint16_t nv_len[NV_SLOTS];

    uint8_t boot_hash[BOOT_HASH_SIZE];  // Pre-stored bootloader hash

    qemu_irq irq;
    SSIBus *ssi;
} S32KTPMState;

#endif