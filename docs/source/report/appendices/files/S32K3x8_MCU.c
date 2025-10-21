#include <stdint.h>
#include <glib.h>
#include "include/hw/arm/S32K3X8EVB.h"
#include "hw/char/S32K_uart.h"
#include "hw/arm/S32K3x8_MCU.h"
#include "hw/arm/S32K3X8EVB.h"
#include "include/hw/ssi/S32K3x8_spi.h"
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

//----------Peripherals------------
//UARTs 
static const uint32_t usart_addr[NXP_NUM_UARTS] = { 
                            0x40328000, //Uart 0 
                            0x4032C000, //Uart 1 
                            0x40330000, //Uart 2
                            0x40334000, //Uart 3
                            0x40338000, //Uart 4
                            0x4033C000, //Uart 5
                            0x40340000, //Uart 6
                            0x40344000, //Uart 7
                            0x4048C000, //Uart 8
                            0x40490000, //Uart 9
                            0x40494000, //Uart 10
                            0x40498000, //Uart 11
                            0x4049C000, //Uart 12
                            0x404A0000, //Uart 13
                            0x404A4000, //Uart 14
                            0x404A8000, //Uart 15
                            };
// IRQ 
static const int usart_irq[NXP_NUM_UARTS] = {   141, //Uart 0 
                                                142, //Uart 1 
                                                143, //Uart 2
                                                144, //Uart 3
                                                145, //Uart 4
                                                146, //Uart 5
                                                147, //Uart 6
                                                148, //Uart 7
                                                149, //Uart 8
                                                150, //Uart 9
                                                151, //Uart 10
                                                152, //Uart 11
                                                153, //Uart 12
                                                154, //Uart 13
                                                155, //Uart 14
                                                156  //Uart 15
                                            };

//------- END UARTs ----------

//------- SPIs ----------
static const uint32_t spi_addr[NXP_NUM_SPI] = { 
    0x40358000, //SPI 0 
    0x4035C000, //SPI 1
    0x40360000, //SPI 2
    0x40364000, //SPI 3
    0x404BC000, //SPI 4
    0x404C0000  //SPI 5 
};

static const int spi_irq[NXP_NUM_SPI] = {
    35, //SPI 0
    36, //SPI 1
    37, //SPI 2
    38, //SPI 3
    39, //SPI 4
    40  //SPI 5
};

//------- END SPIs ----------


//------- MCU initialization -----------
static void S32K3x8_init(Object  *obj){
    S32K3x8State *s = S32K3x8_MCU(obj);
    //Peripheral initialization
    
    //UART
    int i =0;
    for ( i = 0; i < NXP_NUM_UARTS; i++) {
       object_initialize_child(obj, "uart[*]", &s->uart[i],
                                TYPE_S32K3x8_UART);
    }

    //SPI
    for (i = 0; i < NXP_NUM_SPI; i++) {
        object_initialize_child(obj, "spi[*]", &s->spi[i], TYPE_S32K3x8_SPI);
    }

    //cpu initializer
    object_initialize_child(OBJECT(s), "armv7m", &s->cpu,TYPE_ARMV7M);
    //Clock initializer
    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL,0);
}

//------- MCU initialization -----------

//------- MCU realization --------------
static void S32K3x8_realize(DeviceState *dev_mcu, Error **errp){
    S32K3x8State *s = S32K3x8_MCU(dev_mcu);
    
    // Memory
    DeviceState *dev, *armv7m;
    MemoryRegion *system_memory = get_system_memory();
    Error *err = NULL;

    /*if (!s->board_memory) {*/
    /*    error_setg(errp, "memory property was not set");*/
    /*    return;*/
    /*}*/

    /*if (!clock_has_source(s->sysclk)) {*/
    /*    error_setg(errp, "sysclk clock must not be wired up by the board code");*/
    /*    return;*/
    /*}*/

    //Memory Initializer
    // ------------ FLASH --------------- 

    // Init memory addresses
    memory_region_init_rom(
                            &s->flash0, 
                            OBJECT(dev_mcu), 
                            "S32K3.flash",
                            S32K3x8_FLASH0_SIZE, 
                            &err
                          );
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }
    
    memory_region_init_alias(   
                                &s->flash_alias, 
                                OBJECT(dev_mcu),
                                "S32K3.flash.alias", 
                                &s->flash0, 0,
                                S32K3x8_FLASH0_SIZE
                             );
    // ----------- END FLASH ------------
    
    // ------------ RAM -----------------
    memory_region_add_subregion(
                                system_memory, 
                                S32K3x8_FLASH0_BASE, 
                                &s->flash0
                                );
    memory_region_add_subregion(
                                system_memory, 
                                0, 
                                &s->flash_alias);
    memory_region_init_ram( 
                            &s->sram0, 
                            NULL, 
                            "S32K3.sram", 
                            S32K3x8_SRAM0_SIZE,
                            &err
                          );
    
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    memory_region_add_subregion(
                                system_memory, 
                                S32K3x8_SRAM0_BASE, 
                                &s->sram0
                                );
    // ------------ END RAM --------------

    // Attach the parent clock to the MCU
    clock_set_hz(s->sysclk, HCLK_FRQ);

    // CPU class realization
    armv7m = DEVICE(&s->cpu);
    
    // ARM-M7 interrupt setup
    qdev_prop_set_uint32(armv7m, "num-irq", 256);
    /*qdev_prop_set_uint8(armv7m, "num-prio-bits", 4);*/
    
    // CPU description (Useful for QEMU info tree)
    qdev_prop_set_string(armv7m, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m7"));
    /*qdev_prop_set_bit(armv7m, "enable-bitband", true);*/

    // Attach the clock to the CPU
    qdev_connect_clock_in(armv7m, "cpuclk", s->sysclk);\

    // Attach the memory regions to the CPU
    object_property_set_link(
                                OBJECT(&s->cpu), 
                                "memory",
                                OBJECT(system_memory), 
                                &error_abort
                            );
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->cpu),errp)) {
       return; 
    }

    
    // ----------------- UARTs Realization -----------------
    SysBusDevice *busdev;
    /* Attach all UARTs and USART controllers */
    int i = 0;
    for (i = 0; i < NXP_NUM_UARTS; i++) {

        dev = DEVICE(&(s->uart[i]));
        //declare the UART new device type
        qdev_prop_set_chr(dev, "chardev", serial_hd(i));
        if (!sysbus_realize(SYS_BUS_DEVICE(&s->uart[i]), errp)) {
            return;
        }
        busdev = SYS_BUS_DEVICE(dev);
        // Map the UART peripheral to memory
        sysbus_mmio_map(busdev, 0, usart_addr[i]);
        // initialize the IRQ table of the CPU with the correct interrupt handler
        sysbus_connect_irq(busdev, 0, qdev_get_gpio_in(armv7m, usart_irq[i]));
    }

    // ----------------- END UARTs Realization -----------------
    
    // ----------------- SPIs Realization -----------------
    for (i = 0; i < NXP_NUM_SPI; i++) {
        dev = DEVICE(&(s->spi[i]));
        if (!sysbus_realize(SYS_BUS_DEVICE(&s->spi[i]), errp)) {
            return;
        }
        busdev = SYS_BUS_DEVICE(dev);
        // Map the UART peripheral to memory
        sysbus_mmio_map(busdev, 0, spi_addr[i]);
        // initialize the IRQ table of the CPU with the correct interrupt handler
        sysbus_connect_irq(busdev, 0, qdev_get_gpio_in(armv7m, spi_irq[i]));
    }
    // ----------------- END SPIs Realization -----------------
}

//------- END MCU realization --------------

// ------------- MCU properties --------------

static Property S32K3x8_properties[] = {
    DEFINE_PROP_LINK("memory", S32K3x8State, board_memory, TYPE_MEMORY_REGION, MemoryRegion *),
    DEFINE_PROP_UINT32("sram0-size", S32K3x8State, sram0_size, S32K3x8_SRAM0_SIZE),
    DEFINE_PROP_UINT32("flash0-size", S32K3x8State, flash0_size, S32K3x8_FLASH0_SIZE),
    DEFINE_PROP_END_OF_LIST(),
};

// BOARD INIT
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

type_init(S32K3x8_types);
