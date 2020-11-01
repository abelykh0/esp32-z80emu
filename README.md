# esp32-z80emu
Spectrum ZX 48K emulator on VGA-32 board

## Hardware
[VGA32 v1.4 Board](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1083)

## What it can do
* Emulate Spectrum ZX 48K
* Load snapshot in .Z80 format from SD card
* Save snapshot in .Z80 format to SD card
* Output some sounds (only from AY3-8912)

## Third party software
This project uses the following libraries:
* (GPL v3.0) To display video using VGA, process PS/2 keyboard, and output sound: https://github.com/fdivitto/FabGL
* (unsure, MIT?) Z80 emulator: https://github.com/anotherlin/z80emu
* (GPL v2.0) OpenSE Basic: https://spectrumcomputing.co.uk/index.php?cat=96&id=27510

## Plans for the future / issues
* Flickering in some games
* The speed is 12% faster than it is supposed to be
