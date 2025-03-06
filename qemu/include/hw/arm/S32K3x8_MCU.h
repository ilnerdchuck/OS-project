#pragma once

#include <glib.h>
#include "qemu/atomic.h"
#include "glibconfig.h"
#include "hw/sysbus.h"
#include "hw/arm/armv7m.h"
#include "hw/char/S32K_uart.h"
#include "hw/clock.h"
#include "qemu/typedefs.h"
#include "qom/object.h"
#include "hw/arm/S32K3X8EVB.h"
#include "hw/or-irq.h"
#include "hw/arm/armv7m.h"
#include "include/hw/ssi/S32K3x8_spi.h"
#include "qom/object.h"

#define HCLK_FRQ 240000000
#define NXP_NUM_UARTS 16  
#define NXP_NUM_SPI 6  

#define TYPE_S32K3x8_MCU "S32K3x8_MCU"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3x8State, S32K3x8_MCU)

struct S32K3x8State{
    SysBusDevice parent_obj;
    //CPU declaration 
    ARMv7MState cpu;
    
    //CPU clock 
    Clock *sysclk;

    // Memory Setions 
    uint32_t sram0_size;         //SRAM size
    uint32_t flash0_size;        //Flash memory size
    
    //Memory declaration
    MemoryRegion sram0;
    MemoryRegion flash0;
    MemoryRegion flash_alias;
    MemoryRegion *board_memory;
    
    //Main memory
    MemoryRegion container;
    
    //Peripherals 
    S32K3x8UartState uart[NXP_NUM_UARTS];
    S32K3x8SPIState spi[NXP_NUM_SPI];
};

