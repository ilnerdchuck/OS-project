#include <stdint.h>
#include <glib.h>
#include "include/hw/arm/S32K3X8EVB.h"
#include "hw/arm/S32K3x8_MCU.h"
#include "hw/arm/S32K3X8EVB.h"
#include "hw/sysbus.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "hw/arm/boot.h"
#include "exec/address-spaces.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/typedefs.h"
#include "sysemu/sysemu.h"

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "hw/qdev-clock.h"
#include "hw/misc/unimp.h"

static void S32K3x8_init(Object  *obj){
    S32K3x8State *s = S32K3x8_MCU(obj);

    //Clock initializer

    //cpu initializer
    object_initialize_child(OBJECT(s), "armv7m", &s->cpu,TYPE_ARMV7M);
    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL,0);
}

static void S32K3x8_realize(DeviceState *dev_mcu, Error **errp){
    S32K3x8State *s = S32K3x8_MCU(dev_mcu);
    MemoryRegion *system_memory = get_system_memory();
    DeviceState *dev, *armv7m;
    Error *err = NULL;

    /*if (!s->board_memory) {*/
    /*    error_setg(errp, "memory property was not set");*/
    /*    return;*/
    /*}*/

    /*if (!clock_has_source(s->sysclk)) {*/
    /*    error_setg(errp, "sysclk clock must not be wired up by the board code");*/
    /*    return;*/
    /*}*/


    /*object_property_set_link(OBJECT(&s->cpu), "memory", OBJECT(&s->container),&error_abort);*/
    
    
    //memory initializer
    /*memory_region_init(&s->container, dev_mcu, "S32K3x8-container", UINT64_MAX);*/
    //FLash region init 
    memory_region_init_rom(&s->flash0, OBJECT(dev_mcu), "S32K3.flash",S32K3x8_FLASH0_SIZE , &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_init_alias(&s->flash_alias, OBJECT(dev_mcu),
                             "S32K3.flash.alias", &s->flash0, 0,
                             S32K3x8_FLASH0_SIZE);

    memory_region_add_subregion(system_memory, S32K3x8_FLASH0_BASE, &s->flash0);
    memory_region_add_subregion(system_memory, 0, &s->flash_alias);

    memory_region_init_ram(&s->sram0, NULL, "S32K3.sram", S32K3x8_SRAM0_SIZE,
                           &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    memory_region_add_subregion(system_memory, S32K3x8_SRAM0_BASE, &s->sram0);

    clock_set_hz(s->sysclk, HCLK_FRQ);
    armv7m = DEVICE(&s->cpu);
    qdev_prop_set_uint32(armv7m, "num-irq", 32);
    /*qdev_prop_set_uint8(armv7m, "num-prio-bits", 4);*/
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m7"));
    /*qdev_prop_set_bit(armv7m, "enable-bitband", true);*/
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);
    object_property_set_link(OBJECT(&s->cpu), "memory",
                             OBJECT(system_memory), &error_abort);
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->cpu),errp)) {
       return; 
    }
}

static Property S32K3x8_properties[] = {
    DEFINE_PROP_LINK("memory", S32K3x8State, board_memory, TYPE_MEMORY_REGION, MemoryRegion *),
    DEFINE_PROP_UINT32("sram0-size", S32K3x8State, sram0_size, S32K3x8_SRAM0_SIZE),
    DEFINE_PROP_UINT32("flash0-size", S32K3x8State, flash0_size, S32K3x8_FLASH0_SIZE),
    DEFINE_PROP_END_OF_LIST(),
};

static void S32K3x8_class_init(ObjectClass *klass, void *data){
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = S32K3x8_realize;
    device_class_set_props(dc, S32K3x8_properties);
}

static const TypeInfo S32K3x8_info = {
            .name = TYPE_S32K3x8_MCU,
            .parent = TYPE_SYS_BUS_DEVICE,
            .instance_size = sizeof(S32K3x8State),
            .instance_init = S32K3x8_init,
            .class_init = S32K3x8_class_init,
};

static void S32K3x8_types(void){
    type_register_static(&S32K3x8_info);
}

type_init(S32K3x8_types)
