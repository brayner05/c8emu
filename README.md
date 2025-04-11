# CHIP-8 Emulator (c8emu/C8Emu)

## Overview
C8Emu is a lightweight emulator of the CHIP-8 virtual machine, a simple platform used to run programs and games from the 1970s. C8Emu was created as a hobby project with the intention of learning how emulators work, and to review low level programming concepts. Some key features of C8Emu include:

- 4096 bytes of memory with a proper memory-mapped layout
- 16 8-bit general purpose registers (`V0-V7`)
- 1 16-bit register (`I`)
- A scaled graphical output for modern displays

## How it works
C8Emu takes a program as input via a CHIP-8 binary file, which is then loaded into the emulators memory, and executed one instruction at a time. Each instruction is a 2-byte word typically consisting of an opcode and arguments such as registers, literals, and/or memory addresses.

## Prerequisites
Before installing C8Emu, ensure the following are installed on your system:
- [SDL2](https://www.libsdl.org/download-2.0.php) for graphical output
- A C compiler that supports C99 or later
- GNU Make for building the project

## Installation
To install C8Emu, clone this repository.
```bash
git clone https://github.com/brayner05/c8emu.git
```
Then ensure SDL2 is installed, and run:
```bash
make
```
Now that C8Emu is installed, run:
```bash
./bin/c8emu ./demo/game.ch8
```
This will run one of the demo programs provided in the `/demo` directory.

## Usage
As seen above, running a CHIP-8 program on C8Emu is relatively easy. All it takes is running
```bash
c8emu <path-to-program.ch8>
```

