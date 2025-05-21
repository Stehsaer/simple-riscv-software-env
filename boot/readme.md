## Boot Stages

### Stage 0

- All data are stored in ROM. `.data` and `.tdata` must be copied from ROM to RAM after boot.
- No multi-threading and thread-local storage support.
- Stack pointer is initialized to the tail of RAM

#### Usage

Bare-metal test programs

### Stage 0 (Memory Limited Variant)

- Same as Stage 0, but memory usage (`.data`+`.bss`+heap) is limited to 256KiB
- Exception is turned off. No exception info is kept in the program

#### Usage

Bare-metal bootloader that loads program into `RAM_BASE+256KiB` ~ `RAM_BASE+RAM_SIZE`

### Stage 1

- Loaded by bootloader using Stage 0 Memory Limited Variant
- Runs program in `RAM_BASE+256KiB` ~ `RAM_BASE+RAM_SIZE`
- No multi-threading. Thread-local storage is supported.

#### Usage

General purposed programs or RTOS.