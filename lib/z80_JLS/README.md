## z80cpp
#### Z80 core in C++

That's a port from Java to C++ of my [Z80Core](https://github.com/jsanchezv/Z80Core).

To build:
```
mkdir build
cd build
cmake ..
make
```
Then, you have an use case at dir *example*.

The core have the same features of [Z80Core](https://github.com/jsanchezv/Z80Core):

* Complete instruction set emulation
* Emulates the undocumented bits 3 & 5 from flags register
* Emulates the MEMPTR register (known as WZ in official Zilog documentation)
* Strict execution order for every instruction
* Precise timing for all instructions, totally decoupled from the core

*jspeccy at gmail dot com*
