# RutraOS

A 64-bit hobby operating system written from scratch in C and x86\_64 assembly.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Boot Process](#boot-process)
- [Kernel Subsystems](#kernel-subsystems)
  - [Memory Management](#memory-management)
  - [Process Management & Scheduler](#process-management--scheduler)
  - [Shell & Command System](#shell--command-system)
  - [Filesystem (FAT16 / RAM Disk)](#filesystem-fat16--ram-disk)
  - [Drivers](#drivers)
  - [Graphics](#graphics)
- [Available Commands](#available-commands)
- [Project Structure](#project-structure)
- [Building & Running](#building--running)
- [Limitations](#limitations)

---

## Overview

RutraOS boots on any x86\_64 PC (or virtual machine) via GRUB, enters 64-bit long mode, and presents the user with an interactive shell. It is written entirely in C and NASM assembly with no external libraries — no libc, no POSIX, no Linux.

---

## Architecture

```
+-------------------+
|   User Shell      |  ← reads keyboard, parses commands
+-------------------+
|  Command System   |  ← registry of built-in commands
+-------------------+
|  Process Manager  |  ← round-robin scheduler, context switching
+-------------------+
|  Memory Manager   |  ← simple heap (kmalloc / kfree)
+-------------------+
|  Drivers          |  ← VGA, keyboard, mouse, ATA, RTC, timer
+-------------------+
|  Filesystem       |  ← FAT16 on an in-memory RAM disk
+-------------------+
|  Boot (GRUB/ASM)  |  ← Multiboot, long mode setup, GDT, paging
+-------------------+
   x86_64 hardware
```

---

## Boot Process

1. **GRUB** loads the kernel binary (`kernel.bin`) using the Multiboot protocol and jumps to `_start` in `src/boot/boot.asm`.

2. **Assembly bootstrap** (`boot.asm`) runs in 32-bit protected mode and performs:
   - CPUID check — ensures the CPU supports 64-bit long mode.
   - Identity-mapped page tables — maps the first 1 GB using 2 MB huge pages (P4 → P3 → P2).
   - PAE + long-mode bit — sets `CR4.PAE`, `EFER.LME`, and `CR0.PG` to enter long mode.
   - SSE enable — clears `EM`, sets `MP`, `OSFXSR`, and `OSXMMEXCPT` so floating-point works.
   - GDT load — loads a 64-bit Global Descriptor Table and performs a far jump into 64-bit code.

3. **`kernel_main()`** (`src/kernel/kernel.c`) is called from assembly. It:
   - Initialises the VGA terminal and prints a welcome banner.
   - Detects ATA/IDE disks.
   - Initialises the memory heap.
   - Creates and formats a RAM disk with FAT16.
   - Initialises keyboard and mouse drivers.
   - Initialises the VGA graphics driver.
   - Initialises the process manager.
   - Creates two processes — `kernel` (low priority) and `shell` (normal priority).
   - Hands execution to the scheduler, which immediately switches to the shell process.

---

## Kernel Subsystems

### Memory Management

**File:** `src/kernel/memory.c`

RutraOS uses a simple **first-fit heap allocator** backed by a static memory pool in the BSS section.

- `memory_init()` — sets up a single large free block covering the entire pool.
- `kmalloc(size)` — scans the free-block list for the first block that is large enough, splits it if there is excess space (> 16 bytes + header overhead), and marks it as allocated.
- `kfree(ptr)` — marks the block as free and **coalesces** adjacent free blocks to prevent fragmentation.
- Allocations are 4-byte aligned for performance.

The `meminfo` shell command prints heap statistics (allocated / free bytes, block list) and kernel section sizes (`.text`, `.rodata`, `.data`, `.bss`).

---

### Process Management & Scheduler

**Files:** `src/kernel/process.c`, `src/boot/boot.asm` (context switch)

RutraOS supports **cooperative multitasking** with a priority-aware round-robin scheduler.

**Process structure** (`process_t`) holds:
- PID, name, state (`READY` / `RUNNING` / `BLOCKED` / `TERMINATED`)
- Priority (`KERNEL` > `HIGH` > `NORMAL` > `LOW`)
- CPU context (`rax`–`r15`, `rip`, `rsp`, `rbp`, `rflags`, `cr3`)
- Stack and memory allocations

**Scheduler** (`process_schedule`):
1. Walks the circular doubly-linked process list starting after the current process.
2. Picks the ready process with the highest priority (lowest numeric value).
3. Marks the old process as `READY`, the new one as `RUNNING`.
4. Calls `process_switch_context` (implemented in NASM) to save all 64-bit registers and the instruction pointer of the old process and restore them for the new process.

A process voluntarily gives up the CPU by calling `process_yield()`. There is no timer interrupt preemption yet.

The process pool is a fixed-size array of up to 64 process slots allocated at compile time.

---

### Shell & Command System

**Files:** `src/kernel/shell_process.c`, `src/kernel/command.c`

The shell runs as a normal-priority process. Its main loop:
1. Calls `keyboard_getchar()` to block-read one character at a time.
2. Echoes printable characters, handles backspace, and executes on Enter.
3. Passes the input line to `shell_execute_command()`.

**Command parsing** splits the input line into `argc`/`argv` (heap-allocated) by whitespace.

**Command registry** (`command.c`) is a static array of up to 64 `command_info_t` entries, each containing a name, description, and a `main(argc, argv)` function pointer. Commands are registered at shell startup via `command_register_builtins()`. The registry can be queried by name with `command_get_info()`.

---

### Filesystem (FAT16 / RAM Disk)

**Files:** `src/fs/fat16.c`, `src/fs/ramdisk.c`

RutraOS includes an in-memory **RAM disk** formatted with **FAT16**:
- `ramdisk_init()` allocates the disk image in the heap.
- `ramdisk_format_fat16()` writes a valid FAT16 boot sector, FAT tables, and root directory.
- Standard FAT16 operations are available: `ls`, `mkdir`, `create`, `cat`, `cd`, `pwd`, and ramdisk inspection via `ramdisk`.

Real ATA/IDE disks are detected and reported by the `lsdisks` command, but writing to physical disks is not yet implemented.

---

### Drivers

| Driver | File | Description |
|--------|------|-------------|
| Terminal | `src/drivers/terminal.c` | VGA text-mode output — writes characters and attributes directly to the `0xB8000` frame buffer |
| VGA | `src/drivers/vga.c` | Initialises VGA; provides both text mode and (experimental) 320×200 256-colour Mode 13h graphics |
| Keyboard | `src/drivers/keyboard.c` | Reads PS/2 scan codes from I/O port `0x60`, translates them to ASCII |
| Mouse | `src/drivers/mouse.c` | Reads PS/2 mouse packets |
| ATA | `src/drivers/ata.c` | PIO-mode disk detection on primary/secondary IDE channels |
| RTC | `src/drivers/rtc.c` | Reads real-time clock registers for the `date` command |
| Timer | `src/drivers/timer.c` | Programmable Interval Timer (PIT) channel 0 |
| I/O | `src/drivers/io.c` | `inb`/`outb` wrappers for x86 port I/O |

---

### Graphics

**Files:** `src/commands/graphics/gfx.c`, `src/commands/graphics/draw.c`, `src/commands/graphics/fontdemo.c`

The `gfx` command switches the display to **VGA Mode 13h** (320 × 200, 256 colours). In graphics mode you can:
- Draw pixels and filled shapes (`draw` command).
- Display a font demo (`fontdemo`).
- Play a simple side-scrolling **platformer** game (`platformer`).
- Run a Doom-inspired demo (`doom`).

Switching back to text mode restores the VGA terminal.

---

## Available Commands

| Command | Category | Description |
|---------|----------|-------------|
| `help` | Utils | List all available commands |
| `clear` | Utils | Clear the terminal screen |
| `meminfo` | System | Show memory usage and heap statistics |
| `memtest` | System | Run a basic memory allocator test |
| `date` | System | Show current date and time (via RTC) |
| `shutdown` | System | Power off the machine (ACPI/QEMU) |
| `reboot` | System | Reboot via the keyboard controller |
| `lsdisks` | System | List detected ATA/IDE disks |
| `ps` | Process | List running processes |
| `kill <pid>` | Process | Terminate a process by PID |
| `spawn <name>` | Process | Spawn a new process |
| `ls [path]` | Filesystem | List directory contents |
| `mkdir <dir>` | Filesystem | Create a directory |
| `cd <path>` | Filesystem | Change current directory |
| `pwd` | Filesystem | Print working directory |
| `cat <file>` | Filesystem | Print file contents |
| `create <file>` | Filesystem | Create a new file |
| `ramdisk` | Filesystem | Show RAM disk information |
| `gfx` | Graphics | Enter VGA graphics mode |
| `draw` | Graphics | Draw shapes on screen |
| `fontdemo` | Graphics | Font rendering demonstration |
| `platformer` | Graphics | Side-scrolling platformer game |
| `doom` | Graphics | Doom-inspired graphics demo |

---

## Project Structure

```
RutraOS/
├── src/
│   ├── boot/          # NASM bootstrap (Multiboot, long mode, context switch)
│   ├── kernel/        # kernel_main, memory, process, shell, command dispatcher
│   ├── drivers/       # VGA, terminal, keyboard, mouse, ATA, RTC, timer, I/O
│   ├── fs/            # FAT16 implementation + RAM disk
│   ├── lib/           # String utilities
│   ├── include/       # All header files
│   └── commands/
│       ├── filesystem/ # cat, cd, create, ls, mkdir, pwd, ramdisk
│       ├── graphics/   # gfx, draw, fontdemo, doom/, platformer/
│       ├── process/    # kill, ps, spawn
│       ├── system/     # date, lsdisks, meminfo, memtest, reboot, shutdown
│       └── utils/      # clear, help
├── grub.cfg           # GRUB menu entry
├── linker.ld          # Linker script (kernel loaded at 1 MB)
└── Makefile           # Build system
```

---

## Building & Running

### Prerequisites

- `nasm` — Netwide Assembler
- `gcc` — GCC configured for freestanding `elf64` targets
- `ld` — GNU linker
- `grub-mkrescue` + `xorriso` — to create the bootable ISO
- `qemu-system-x86_64` — to run the ISO in a VM

### Commands

```bash
# Build the bootable ISO
make

# Build and launch in QEMU
make run

# Remove build artefacts
make clean

# Full rebuild
make rebuild
```

The build output is `build/os.iso`.

---

## Limitations

- No UEFI support — BIOS/Multiboot only.
- No interrupt-driven preemption — scheduling is cooperative (`process_yield()`).
- No memory protection — all processes share the same address space and page tables.
- No physical disk writes — ATA driver is detection/read only.
- No networking.
- California residents may no longer use RutraOS after 01.01.2027.
- Colorado residents may no longer use RutraOS after 01.01.2028.
- This operating system will not have age verification.
