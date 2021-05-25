# z80 Emulation Library

I wrote this z80 emulator library in 2008 after being dissatisfied with other
emulators for one reason or another. It aims to be cycle-accurate by modeling
the number of T-cycles per instruction. See the [Z80 CPU User
Manual](https://www.zilog.com/docs/z80/um0080.pdf) for details about T-cycles.

## Operation

Libzel is very low level. It operates one instruction at a time, calling
callbacks to read memory, perform I/O, and handle interrupts. No assumptions
are made about memory layout or peripheral organization.

None of the standard Zilog peripherals are implemented and they must be
handled by the callbacks.

## Paper

Libzel was used to emulate the Sequoia AVC Advantage voting machine as
described in Checkoway et al.'s [_Can DREs Provide Long-Lasting Security? The
Case of Return-Oriented Programming and the AVC
Advantage_](https://www.cs.uic.edu/~s/papers/evt2009/).
