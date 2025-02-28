#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "hw/arm/boot.h"
#include "exec/address-spaces.h"
#include "hw/arm/stm32f205_soc.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/typedefs.h"
#include "sysemu/sysemu.h"
#include "include/hw/arm/S32K3X8EVB.h"
#include "hw/arm/S32K3x8_MCU.h"
#include "hw/arm/S32K3X8EVB.h"

static void S32K3x8_init(Object  *obj){
    S32K3x8State *s = S32K3x8_MCU(obj);

    //Clock initializer
    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL,0);

    //memory initializer
    memory_region_init(&s->container, obj, "S32K3x8-container", UINT64_MAX);

    //FLASH 
    /* Create the Flash memory ROM region. */
    Error *err, *errp = NULL;

    memory_region_init_rom(&s->flash0, OBJECT(s), "S32K3.flash", S32K3x8_FLASH0_SIZE , &err);
    if (err) {
        error_propagate(&errp, err);
        
        return;
    }
    /* Map the Flash memory. */
    memory_region_add_subregion(&s->container, S32K3x8_FLASH0_BASE, &s->flash0);
    /* Initialize SRAM memory region. */
    memory_region_init_ram(&s->sram0, OBJECT(s), "SK32K3.sram", S32K3x8_SRAM0_SIZE, &err);
    if (err) {
        error_propagate(&errp, err);
        return;
    }
    /* Map the SRAM memory region. */
    memory_region_add_subregion(
        &s->container,
        S32K3x8_SRAM0_BASE,
        &s->sram0
    );
    //cpu initializer
    object_initialize_child(OBJECT(s), "armv7m", &s->cpu,TYPE_ARMV7M);
    qdev_prop_set_string(DEVICE(&s->cpu), "cpu-type",ARM_CPU_TYPE_NAME("cortex-m7"));
    qdev_prop_set_uint32(DEVICE(&s->cpu), "num-irq", 32); //TODO: check how may irqs are supported
}

static void S32K3x8_realize(DeviceState *dev_mcu, Error **errp){
    S32K3x8State *s = S32K3x8_MCU(dev_mcu);
    Error *err = NULL;

    if (!s->board_memory) {
        error_setg(errp, "memory property was not set");
        return;
    }

    if (clock_has_source(s->sysclk)) {
        error_setg(errp, "sysclk clock must not be wired up by the board code");
        return;
    }

    clock_set_hz(s->sysclk, HCLK_FRQ);
    qdev_connect_clock_in(DEVICE(&s->cpu), "cpuclk", s->sysclk);

    /* Initialize SRAM memory region. */
    memory_region_init_ram(&s->sram0, OBJECT(s), "S32K3x8.sram0", s->sram0_size, &err);
    if (err) {
        error_propagate(errp, err);
        return;
    }

    /* Map the SRAM memory region in our memory container at the correct base address. */
    memory_region_add_subregion(&s->container, S32K3x8_SRAM0_BASE, &s->sram0);

    //Link container main memory to system memory.
    object_property_set_link(OBJECT(&s->cpu), "memory", OBJECT(&s->container),&error_abort);
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
