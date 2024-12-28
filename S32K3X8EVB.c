#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"
#include "hw/boards.h"
#include "hw/arm/boot.h"
#include "sysemu/sysemu.h"
#include "exec/address-spaces.h"
#include "qom/object.h"

//#include "hw/arm/samd21_mcu.h"
//#include "hw/i2c/microbit_i2c.h"
//#include "hw/qdev-properties.h"


// Device class for our board

struct S32K3X8EVBMachineState {
    /* Parent machine state. */
    MachineState parent;
};


#define TYPE_S32K3X8EVB_MACHINE MACHINE_TYPE_NAME("S32K3X8EVB")
OBJECT_DECLARE_SIMPLE_TYPE(S32K3X8EVBMachineState, S32K3X8EVB_MACHINE)

static void S32K3X8EVB_init(MachineState *machine){
}
    

// QEMU machine init     
static void S32K3X8EVB_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "NXP S32K3X8EVB-Q289";
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

