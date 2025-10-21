# Group 10 QEMU implementation of the NXP S32K3X8EVB

This project aims to develop an implementation of the NXP S32K3X8EVB board in QEMU to provide
a environment to develop applications on the given board. The project doesn't implement the whole
board but only the basic memories and a couple of peripherals: UART and SPI.
Furthermore to test the implementation a port of FreeRTOS is made using the NXP gcc toolchain.

## Project structure

The project is split in a couple of folders:

- FreeRTOS_App: containing the application used to test the board and the peripherals
- gcc-10.2.0: NXP toolchain
- material: documents regarding the board functionalities and implementation 
- qemu: modified qemu v9.0 with the added board
- docs: further documentation about the project and the presentation

## Qemu structure

Qemu was modified in two points

## Building QEMU

- To compile qemu first you need to install some dependencies:
- If on Ubuntu/Debian:

```
sudo apt update
sudo apt install build-essential zlib1g-dev libglib2.0-dev \
    libfdt-dev libpixman-1-dev ninja-build python3-sphinx \
    python3-sphinx-rtd-theme pkg-config libgtk-3-dev \
    libvte-2.91-dev libaio-dev libbluetooth-dev \
    libbrlapi-dev libbz2-dev libcap-dev libcap-ng-dev \
    libcurl4-gnutls-dev python3-venv gcc-arm-none-eabi cmake
```

- If on Fedora/RHL:

```
    sudo dnf update
    sudo dnf install ... TDB (is a bit strange, it varies too much distro by distro)
```

- Move into the qemu build folder:

```
    cd group10
    cd qemu/build
```

- Configure the target architectures:
This is useful if you want only to compile the arm boards/cpus

```
    ../configure --target-list=arm-softmmu
```

- Build the new QEMU executable

```
    make -j$(nproc)
```

- To ceck if the board is available and correctly registered inside the QEMU environment

```
    ./qemu-system-arm -machine help
```

to list all the available machines

## Running QEMU

To run QEMU with the implemented board we simply run

```
    ./qemu/build/qemu-system-arm -machine S32K3X8EVB -monitor stdio -m 128M
```

## Running FreeRTOS on QEMU

To run FreeRTOS applications a Makefile has been made inside the `./FreeRTOS_App` folder. Three are the main commands:
 - `make`: used to compile the FreeRTOS application/firmware
 - `qemu_start`: to start QEMU with the implemented board and the last compiled firmware
 - `qemu_debug`: to start QEMU in debug mode with the implemented board and the last compiled firmware
 - `gdb_start`: to start a gdb session that automatically attaches to the `qemu_debug` session. NOTE: on Ubuntu/Debian gdb-multiarch is required, meanwhile on Fedora/RHEL a fork of gdb is needed. Furthermore the GEF plugin is suggested to improve the debug capabilities
 
## Material

- <https://lwn.net/Kernel/LDD3/>
- <https://www.qemu.org/docs/master/system/target-arm.html#arm-system-emulator>
- <https://elixir.bootlin.com/qemu/v9.1.2/source>
- <https://medium.com/@matanbach44/hello-world-device-in-qemu-ae69b02872f4>
- <https://s-o-c.org/simulating-arm-cortex-m-with-qemu-tips/>
- <https://stackoverflow.com/help/minimal-reproducible-example>
- <https://www.qemu.org/docs/master/system/arm/musca.html>
- <https://airbus-seclab.github.io/qemu_blog/machine.html>
- <https://github.com/airbus-seclab/qemu_blog/blob/main/README.md>
- <https://github.com/airbus-seclab/qemu_blog/blob/main/machine.md>
- <https://wiki.qemu.org/Documentation/QOMConventions>
- <https://github.com/qemu/qemu/blob/master/hw/arm/stellaris.c>
- <https://github.com/qemu/qemu>
- <https://github.com/quarkslab/sstic-tame-the-qemu/blob/b002e5033e70aa881daed95f92285916ac008a07/qemu/hw/arm/samd21.c#L286>
- <https://www.nxp.com/docs/en/application-note/AN5413.pdf#G1027000>

# Authors

 - Francesco Mignone (s336801)
 - Leonardo Gallina (s293704 --> s349363)
 - Andrea Baraldi (s339846)
 - Silvia Bonenti (s339232)
 - Lorenzo Parata (s343744)

[Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)](https://creativecommons.org/licenses/by/4.0/)
