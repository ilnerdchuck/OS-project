#pragma once

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/arm/boot.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "qom/object.h"
#include "hw/qdev-clock.h"
#include "hw/arm/S32K3X8EVB.h"
#include "hw/arm/S32K3x8_MCU.h"

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/error-report.h"
#include "hw/arm/boot.h"

//TODO: define all memory regions
//FLASH
//TODO: ATTENZIONE DA FARE PRESENTE AL PROF S32K356 non ha flash 3
//TODO: sia della flash che della ram ci sono piu blocchi 
//che facciamo li dividiamo?

#define S32K3x8_FLASH0_BASE 0x00400000
#define S32K3x8_FLASH0_SIZE 2048*1024

//SRAM
#define S32K3x8_SRAM0_BASE 0x20400000
#define S32K3x8_SRAM0_SIZE 256*1024


#define TYPE_S32K3X8EVB_MACHINE MACHINE_TYPE_NAME("S32K3X8EVB")
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8EVBMachineState, S32K3X8EVB_MACHINE)

// Device class for our board

struct S32K3X8EVBMachineState {
    /* Parent machine state. */

    MachineState parent;
    
    S32K3x8State S32K3X8;
};
