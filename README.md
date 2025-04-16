# Simple RISCV Software Environment

A simple 32-bit RISCV software environment, written in C++ and assembly, aiming to provide a simple environment for baremetal programs on a baremetal RISCV platform and bootloaders in non-OS environments.

## Prerequisite

### RISCV GNU Toolchain

The project depends on [RISCV GNU Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) to function. 

- Follow the instructions and install the `newlib` (`riscv32/64-unknown-elf-xxx`) version. 
- Make sure the location is accessible through `PATH`. Type:

  ```bash
  riscv64-unknown-elf-gcc --version
  ```

  and you should get the following output (version string may vary):

  ```
  riscv64-unknown-elf-gcc (g04696df09633-dirty) 14.2.0
  Copyright (C) 2024 Free Software Foundation, Inc.
  This is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ```

### Xmake

This project uses Xmake as the building system and package manager. Get Xmake from [here](https://github.com/xmake-io/xmake).

### QEMU

This is somewhat optional, but if you want to use/run/refer to the subprojects for the QEMU platform, you need to get QEMU for RISC-V.

## Getting Started

### Step 1

For host systems that uses **bash**, execute `./configure release` to quickly configure the project. For **Windows** without **bash**, manually execute the command in the script (`configure`), replacing `$1` with `release` or other desired mode.

Refer to [Xmake documentation](https://xmake.io/#/manual/custom_rule?id=built-in-rules) for further information on *modes*.

### Step 2

Build the project using `xmake build`.

### Step 3

For sub-projects built for QEMU, use `xmake run` to see the results. For sub-projects build for "kintex7", see the descriptions below.

## Project Structure

### Modules

`Modules` provides the basic environment in a modular manner. Combine different modules together to support different features. Examples are:

- Dynamic Allocation: `allocator`
- Emulated File System: `file.interface`
- Fatfs: `file.driver.fatfs`
  - Additional SD card backend: `file.driver.fatfs.backend.sd`

However some modules are essential:

- `device`: Provides class/interfaces to different device across different platforms.
- `platform`: Provides platform-specific definitions for devices, eg. Clocks, SPI, Serial...
- `start.*`: Provides startup assembly programs that kick-starts the platform by setting up Stack Pointers, copying data from LMA to VMA etc.

#### Platforms

##### Kintex7

"Kintex7" refers to the FPGA chip model `7k410t-ffg900-2` I'm using for my CPU design projects. Without the Vivado project file / block-design file, it may not be easy to figure out the exact usage of the sub-projects. Progress is being made to make this publicly accessible. For details see [my CPU design project](https://github.com/Stehsaer/simple-riscv-cpu-design) and also the device definitions and their corresponding addresses.

##### QEMU

Sub-projects for QEMU platforms are mainly for the college course "Operating System" I currently take (by Mar. 2025). QEMU documentations are publicly available.

### Projects

This project also contains some sub-projects for different purposes.

- `kintex7-test`: for testing purposes only. May exclude from the repository in the future.
- `os-class`: for lab assignments from my Operating System course. (Side-note: the course documentation is written for X86, but it also allows for other ISAs and languages, so I decided to go with RISC-V and C++)