#pragma once

#include "qemu/atomic.h"
#include "glib.h"
#include "glibconfig.h"
#include "hw/sysbus.h"
#include "hw/arm/armv7m.h"
#include "hw/clock.h"
#include "qemu/typedefs.h"
#include "qom/object.h"
#include "hw/arm/S32K3X8EVB.h"

//Clock freq TODO: quale cavolo é la nostra dato che l'S32k3 arriva fino a 320?
#define HCLK_FRQ 120000000


#define TYPE_S32K3x8_MCU "S32K3x8_MCU"
OBJECT_DECLARE_SIMPLE_TYPE(S32K3x8State, S32K3x8_MCU)

struct S32K3x8State{
    SysBusDevice parent_obj;
    //CPU declaration 
    ARMv7MState cpu;
    /* CPU clock. */
    Clock *sysclk;

    uint32_t sram0_size;         /* SRAM size. */
    uint32_t flash0_size;        /* Flash memory size */
    //Memory declaration
    MemoryRegion sram0;
    MemoryRegion flash0;
    MemoryRegion *board_memory;
    //Main memory
    MemoryRegion container;
    
};

