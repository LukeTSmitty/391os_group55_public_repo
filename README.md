# ECE 391 OS — Group 55

A general-purpose operating system built from scratch in C, targeting a 64-bit RISC-V virtual machine. The kernel runs in QEMU's `virt` environment and supports real multi-process workloads through a Unix-inspired design. 

Full implementation kept private to abide by academic integrity guidelines. Detailed walkthrough, architecture discussion, and code review available by request or during an interview. Feel free to message me on Linkedin (https://www.linkedin.com/in/luke-smith-500730377/)

---

## Overview

This project implements the core layers of a modern OS kernel: virtual memory, process and thread management, interrupt handling, device drivers, an inode-based filesystem, and a Unix-like system call interface. A user-space shell sits on top and can launch ELF binaries, chain commands with pipes, and interact with the filesystem.

---

## Architecture

**Target platform:** RISC-V 64-bit (`rv64imazicsr`), QEMU `virt` machine  
**Language:** C with minimal RISC-V assembly  
**Build system:** GNU Make with a RISC-V cross-compiler toolchain  

The kernel enters S-mode (supervisor) after M-mode boot setup and from there owns memory protection, trap handling, and scheduling. User programs run in U-mode with no direct hardware access.

---

## Key Subsystems

### Virtual Memory
- Sv39 paged virtual address translation (39-bit VA space, 4 KB pages)
- Per-process isolated address spaces managed via the `SATP` CSR
- Kernel heap allocator (`kmalloc` / `kfree`)
- U-mode page fault handling for demand paging

### Process & Thread Management
- Up to 16 concurrent processes, each with an independent memory space
- Lightweight threads (up to 32 per process) with preemptive round-robin scheduling
- `fork` / `exec` / `exit` / `wait` semantics
- ELF binary loader for user-space executables
- Synchronization primitives: condition variables, recursive locks, atomics

### Interrupt & Exception Handling
- PLIC (Platform-Level Interrupt Controller) with 96 interrupt sources
- Vectored trap dispatch for exceptions, syscalls, and device interrupts
- Supervisor timer interrupts drive preemption (10 MHz base clock)

### Device Drivers
| Driver | Interface |
|---|---|
| UART (NS8250) | Serial console, interrupt-driven |
| VirtIO Block (`vioblk`) | Virtual disk, virtqueue-based |
| VirtIO RNG (`viorng`) | Kernel entropy source |

### Filesystem (KTFS)
- Custom inode-based filesystem on top of the block device
- 512-byte blocks with direct, indirect, and doubly-indirect block pointers
- 64-block LRU write-back block cache

### System Call Interface
| Category | Calls |
|---|---|
| Process control | `exit`, `exec`, `fork`, `wait` |
| I/O | `read`, `write`, `close`, `ioctl` |
| Filesystem | `fsopen`, `fscreate`, `fsdelete` |
| IPC | `pipe`, `iodup` |
| Misc | `usleep`, `devopen` |

### Inter-Process Communication
Kernel pipe implementation connected processes through a shared ring buffer

---

## User Space

A Unix-inspired shell (`shell.elf`) bootstraps the user environment. Pre-built binaries that run on the OS include standard I/O utilities, a Fibonacci demo, and classic text adventures (Zork, Rogue, Trek).


