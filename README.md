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

In THEORY, you should be able to build this project using any RISCV GCC that supports C++23. Currently GCC 14.2.0 and GCC 15.1.0 are known to work.

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

### Baremetal Modules

Baremetal modules provides the basic supporting environment for baremetal execution in a modular manner. Combine different modules together to support different features.

- `baremetal.allocator`: Heap allocator. Needs `_heap_start` and `_heap_end` from linkerscript
- `baremetal.filesystem`: Simple filesystem to mount devices and support POSIX APIs
- `baremetal.mutex`: Simple mutex implementation
- `baremetal.platform`: Contains platform-specific code that implements the baremetal modules
  - Kintex7 Platform: supports SD card file accessing and UART serial input/output
  - QEMU Platform: supports reading from virtual disk and UART serial input/output. Writing to virtual disk is broken and doesn't work.
- `baremetal.time`: Provides time query functions for Newlib and implements 2 spin-locked sleep functions (`sleep` and `usleep`).

### Platforms

#### Kintex7

"Kintex7" refers to the FPGA chip model `7k410t-ffg900-2` I'm using for my CPU design projects. Without the Vivado project file / block-design file, it may not be easy to figure out the exact usage of the sub-projects. Progress is being made to make this publicly accessible. For details see https://github.com/Stehsaer/simple-riscv-cpu-design and also the device definitions and their corresponding addresses.

#### QEMU

QEMU platforms are mainly for the college course "Operating System" I currently take (by Mar. 2025). QEMU documentations are publicly available. This project uses the `virt` platform of RISC-V QEMU.

### Projects

This project also contains some sub-projects for different purposes.

- `os-class`: for lab assignments from my Operating System course. (Side-note: the course documentation is written for X86, but it also allows for other ISAs and languages, so I decided to go with RISC-V and C++)