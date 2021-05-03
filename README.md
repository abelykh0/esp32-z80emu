# esp32-z80emu
Spectrum ZX 128K emulator on VGA-32 board

## Hardware
[VGA32 v1.4 Board](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1083)

## What it can do
* Emulate Spectrum ZX 128K
* Load snapshot in .Z80 format from SD card
* Save snapshot in .Z80 format to SD card
* Output some sounds (only from AY3-8912)
* Kempston mouse
* Load ROMs from SD card (`/roms/128-0.rom`; `/roms/128-1.rom`. Fall back to OpenSE Basic if not present)

## Third party software
This project uses the following libraries:
* (GPL v3.0) Display video using VGA, process PS/2 keyboard, and sound output: https://github.com/fdivitto/FabGL
* (GPL v2.0) OpenSE Basic: https://spectrumcomputing.co.uk/index.php?cat=96&id=27510

Choose one of the following for Z80 CPU emulator:
* (unsure, MIT?) Lin Ke-Fong's https://github.com/anotherlin/z80emu
* (zlib License) Andre Weissflog's https://github.com/floooh/chips/
* (GPL v3.0) José Luis Sánchez's https://github.com/jsanchezv/z80cpp

## Plans for the future / issues
* Beeper sounds
* Flickering in some games
* The speed is 12% faster than it is supposed to be
* Load files from https://spectrumcomputing.co.uk/
