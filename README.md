# esp32-z80emu
Spectrum ZX 128K emulator on VGA-32 board

## Hardware
[VGA32 v1.4 Board](https://www.lilygo.cc/en-ca/products/fabgl-vga32)

## What it can do
* Emulate Spectrum ZX 128K
* Load snapshot in .Z80 format from SD card
* Save snapshot in .Z80 format to SD card
* Output some sounds (partial support for AY3-8912)
* Kempston mouse
* Load ROMs from SD card (`/roms/128-0.rom`; `/roms/128-1.rom`. Fall back to OpenSE Basic if not present)
* Not using any PSRAM

<a href="http://www.youtube.com/watch?feature=player_embedded&v=OEjMUaaSh-4
" target="_blank"><img src="https://i.ytimg.com/vi/OEjMUaaSh-4/hqdefault.jpg" 
alt="ZX Spectrum Emulator on ESP32" width="480" height="360" border="10" /></a>

## Third party software
This project uses the following libraries:
* (GPL v3.0) Display video using VGA, process PS/2 keyboard, and sound output: https://github.com/fdivitto/FabGL
* (GPL v2.0) OpenSE Basic: https://spectrumcomputing.co.uk/index.php?cat=96&id=27510

Choose one of the following for Z80 CPU emulator:
* (One liner, for those who don't like reading long legal documents) Lin Ke-Fong's https://github.com/anotherlin/z80emu
* (GPL v3.0) José Luis Sánchez's https://github.com/jsanchezv/z80cpp
* (MIT) Steve Checkoway's https://github.com/stevecheckoway/libzel  
* (zlib License) Andre Weissflog's https://github.com/floooh/chips (this one seems too slow)

## Plans for the future / issues
* Flickering in some games
* Beeper
* Support noise and envelope for AY3-8912 sound
* The speed is 12% faster than it is supposed to be

