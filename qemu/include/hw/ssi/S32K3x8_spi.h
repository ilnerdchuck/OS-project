#ifndef HW_STM32F2XX_SPI_H
#define HW_STM32F2XX_SPI_H

#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qom/object.h"

#define STM_SPI_CR1     0x00
#define STM_SPI_CR2     0x04
#define STM_SPI_SR      0x08
#define STM_SPI_DR      0x0C
#define STM_SPI_CRCPR   0x10
#define STM_SPI_RXCRCR  0x14
#define STM_SPI_TXCRCR  0x18
#define STM_SPI_I2SCFGR 0x1C
#define STM_SPI_I2SPR   0x20

#define STM_SPI_CR1_SPE  (1 << 6)
#define STM_SPI_CR1_MSTR (1 << 2)

#define STM_SPI_SR_RXNE   1

#define TYPE_S32K3x8_SPI "S32K3x8_SPI"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3x8SPIState, S32K3x8_SPI)

struct S32K3x8SPIState {
    /* <private> */
    SysBusDevice parent_obj;

    /* <public> */
    MemoryRegion mmio;

    uint32_t spi_cr1;
    uint32_t spi_cr2;
    uint32_t spi_sr;
    uint32_t spi_dr;
    uint32_t spi_crcpr;
    uint32_t spi_rxcrcr;
    uint32_t spi_txcrcr;
    uint32_t spi_i2scfgr;
    uint32_t spi_i2spr;

    qemu_irq irq;
    SSIBus *ssi;
};

#endif 
