
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/ssi/S32K3x8_spi.h"
#include "migration/vmstate.h"

#ifndef S32_SPI_ERR_DEBUG
#define S32_SPI_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (S32_SPI_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0)

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void S32Kx8_spi_reset(DeviceState *dev)
{
    S32K3x8SPIState *s = S32K3x8_SPI(dev);

    s->spi_cr1 = 0x00000000;
    s->spi_cr2 = 0x00000000;
    s->spi_sr = 0x0000000A;
    s->spi_dr = 0x0000000C;
    s->spi_crcpr = 0x00000007;
    s->spi_rxcrcr = 0x00000000;
    s->spi_txcrcr = 0x00000000;
    s->spi_i2scfgr = 0x00000000;
    s->spi_i2spr = 0x00000002;
}
// Function to handle SPI data transfer
static void S32Kx8_spi_transfer(S32K3x8SPIState *s)
{
    DB_PRINT("Data to send: 0x%x\n", s->spi_dr);

    s->spi_dr = ssi_transfer(s->ssi, s->spi_dr);
    // Clear the RXNE flag and set it again to indicate that new data is available
    s->spi_sr |= S32_SPI_SR_RXNE;

    DB_PRINT("Data received: 0x%x\n", s->spi_dr);
}

static uint64_t S32Kx8_spi_read(void *opaque, hwaddr addr,
                                     unsigned int size)
{
    S32K3x8SPIState *s = opaque;

    DB_PRINT("Address: 0x%" HWADDR_PRIx "\n", addr);

    switch (addr) {
    case S32_SPI_CR1:
        return s->spi_cr1;
    case S32_SPI_CR2:
        qemu_log_mask(LOG_UNIMP, "%s: Interrupts and DMA are not implemented\n",
                      __func__);
        return s->spi_cr2;
    case S32_SPI_SR:
        return s->spi_sr;
    case S32_SPI_DR:
        S32Kx8_spi_transfer(s);
        s->spi_sr &= ~S32_SPI_SR_RXNE;
        s->spi_dr |= 0xFFFFFF66;
        return s->spi_dr;
    case S32_SPI_CRCPR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers \n", __func__);
        return s->spi_crcpr;
    case S32_SPI_RXCRCR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers \n", __func__);
        return s->spi_rxcrcr;
    case S32_SPI_TXCRCR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented, the registers \n", __func__);
        return s->spi_txcrcr;
    case S32_SPI_I2SCFGR:
        qemu_log_mask(LOG_UNIMP, "%s: I2S is not implemented, the registers \n", __func__);
        return s->spi_i2scfgr;
    case S32_SPI_I2SPR:
        qemu_log_mask(LOG_UNIMP, "%s: I2S is not implemented, the registers \n", __func__);
        return s->spi_i2spr;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%" HWADDR_PRIx "\n",
                      __func__, addr);
    }

    return 0;
}

static void S32Kx8_spi_write(void *opaque, hwaddr addr,
                                uint64_t val64, unsigned int size)
{
    S32K3x8SPIState *s = opaque;
    uint32_t value = val64;

    DB_PRINT("Address: 0x%" HWADDR_PRIx ", Value: 0x%x\n", addr, value);

    switch (addr) {
    case S32_SPI_CR1:
        s->spi_cr1 = value;
        return;
    case S32_SPI_CR2:
        qemu_log_mask(LOG_UNIMP, "%s: " \
                      "Interrupts and DMA are not implemented\n", __func__);
        s->spi_cr2 = value;
        return;
    case S32_SPI_SR:
        /* Read only register, except for clearing the CRCERR bit, which
         * is not supported
         */
        return;
    case S32_SPI_DR:
        s->spi_dr = value;
        S32Kx8_spi_transfer(s);
        return;
    case S32_SPI_CRCPR:
        qemu_log_mask(LOG_UNIMP, "%s: CRC is not implemented \n", __func__);
        return;
    case S32_SPI_RXCRCR:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Read only register: " \
                      "0x%" HWADDR_PRIx "\n", __func__, addr);
        return;
    case S32_SPI_TXCRCR:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Read only register: " \
                      "0x%" HWADDR_PRIx "\n", __func__, addr);
        return;
    case S32_SPI_I2SCFGR:
        qemu_log_mask(LOG_UNIMP, "%s: " \
                      "I2S is not implemented\n", __func__);
        return;
    case S32_SPI_I2SPR:
        qemu_log_mask(LOG_UNIMP, "%s: " \
                      "I2S is not implemented\n", __func__);
        return;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%" HWADDR_PRIx "\n", __func__, addr);
    }
}

static const MemoryRegionOps S32Kx8_spi_ops = {
    .read = S32Kx8_spi_read,
    .write = S32Kx8_spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_S32Kx8_spi = {
    .name = TYPE_S32K3x8_SPI,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (const VMStateField[]) {
        VMSTATE_UINT32(spi_cr1, S32K3x8SPIState),
        VMSTATE_UINT32(spi_cr2, S32K3x8SPIState),
        VMSTATE_UINT32(spi_sr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_dr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_crcpr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_rxcrcr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_txcrcr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_i2scfgr, S32K3x8SPIState),
        VMSTATE_UINT32(spi_i2spr, S32K3x8SPIState),
        VMSTATE_END_OF_LIST()
    }
};

static void S32Kx8_spi_init(Object *obj)
{
    S32K3x8SPIState *s = S32K3x8_SPI(obj);
    DeviceState *dev = DEVICE(obj);

    memory_region_init_io(&s->mmio, obj, &S32Kx8_spi_ops, s,
                          TYPE_S32K3x8_SPI, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);

    s->ssi = ssi_create_bus(dev, "ssi");
}

static void S32Kx8_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    device_class_set_legacy_reset(dc, S32Kx8_spi_reset);
    dc->vmsd = &vmstate_S32Kx8_spi;
}

static const TypeInfo S32Kx8_spi_info = {
    .name          = TYPE_S32K3x8_SPI,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K3x8SPIState),
    .instance_init = S32Kx8_spi_init,
    .class_init    = S32Kx8_spi_class_init,
};

static void S32Kx8_spi_register_types(void)
{
    type_register_static(&S32Kx8_spi_info);
}

type_init(S32Kx8_spi_register_types)
