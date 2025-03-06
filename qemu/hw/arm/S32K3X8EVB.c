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
//#include "hw/arm/samd21_mcu.h"
//#include "hw/i2c/microbit_i2c.h"
//#include "hw/qdev-properties.h"


// Device class for our board

struct S32K3X8EVBMachineState {
    /* Parent machine state. */
    MachineState parent;
    
    S32K3x8State S32K3X8;
};


#define TYPE_S32K3X8EVB_MACHINE MACHINE_TYPE_NAME("S32K3X8EVB")
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8EVBMachineState, S32K3X8EVB_MACHINE)

static void S32K3X8EVB_init(MachineState *machine){
    DeviceState *dev;
    Clock *sysclk;

    /* This clock doesn't need migration because it is fixed-frequency */
    sysclk = clock_new(OBJECT(machine), "SYSCLK");
    clock_set_hz(sysclk, HCLK_FRQ);

    dev = qdev_new(TYPE_S32K3x8_MCU);
    object_property_add_child(OBJECT(machine), "soc", OBJECT(dev));
    qdev_connect_clock_in(dev, "sysclk", sysclk);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);

    armv7m_load_kernel(ARM_CPU(first_cpu), machine->kernel_filename,
                       0, S32K3x8_FLASH0_SIZE);
    

}
    

// QEMU machine init     
static void S32K3X8EVB_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "NXP S32K3X8EVB-Q289 (Cortex-M7)";
    mc->init = S32K3X8EVB_init;
    mc->max_cpus = 1;
}

/*
  The following structure describes the machine class type:
    - It sets the machine name ("S32K3X8EVB").
    - It sets the parent class type (a QEMU machine).
    - It sets the instance size.
    - It sets the class initialization callback.
*/
static const TypeInfo S32K3X8EVB_info = {
    .name = TYPE_S32K3X8EVB_MACHINE,
    .parent = TYPE_MACHINE,
    .instance_size = sizeof(S32K3X8EVBMachineState),
    .class_init = S32K3X8EVB_machine_class_init,
};

static void S32K3X8EVB_machine_init(void){
    type_register_static(&S32K3X8EVB_info);
}

type_init(S32K3X8EVB_machine_init);

