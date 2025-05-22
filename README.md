[![PlatformIO CI](https://github.com/xboxoneresearch/PicoDurangoPOST/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/xboxoneresearch/PicoDurangoPOST/actions/workflows/build.yml)
![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/xboxoneresearch/PicoDurangoPOST/latest/total)
[![GitHub latest Tag](https://img.shields.io/github/v/tag/xboxoneresearch/PicoDurangoPOST)](https://github.com/xboxoneresearch/PicoDurangoPOST/releases/latest)

# Durango POST-Code reader via RasPi Pico

This software flashed to the Raspberry Pi Pico allows for monitoring of POST codes, which is useful for fault diagnosis on the hardware.

Jump to [Usage](#usage)

Technical infos:

- [POST bus on Xbox One - Wiki page](https://xboxoneresearch.github.io/wiki/hardware/post/)

- [List of error codes](https://errors.xboxresearch.com)

Also check out the videos from @craftbenmine

[![craftbenmine YouTube video](https://img.youtube.com/vi/BuPhxKdxU0g/hqdefault.jpg)](https://www.youtube.com/watch?v=BuPhxKdxU0g)

and @ACE Console Repairs

[![ACE Console Repairs Video](https://img.youtube.com/vi/qWvvBrVMNzk/hqdefault.jpg)](https://www.youtube.com/watch?v=qWvvBrVMNzk)

## Supported consoles

- ✅ Xbox One PHAT
- ✅ Xbox One S
- ✅ Xbox One X

Additional infos in preparation, but generally also supported:

- ⚠️ Xbox Series S
- ⚠️ Xbox Series X

## Connections

> [!CAUTION]
> LOOK AT THE PIN MARKINGS ON THE FACET HEADER before soldering!
>
> Pin 1 is marked with a white dot, pins 2, 25, 26 by numbers!
>
> FACET connector orientation DOES CHANGE between console revisions!

> [!IMPORTANT]
> Xbox Series S / X require additional components to be populated, to activate I2C over FACET. More infos on that will follow!

Pi Pico -> FACET

- SDA: Pi Pico **Pin 1** (GP0) -> FACET **Pin 26**
- SCL: Pi Pico **Pin 2** (GP1) -> FACET **Pin 25**
- GND -> GND

![Pi Pico Facet I2C connection diagram](./assets/connection_diagram.png)

![Pi Pico Facet I2C diagram - all revs](./assets/all_revs_diagram_ACE.jpg)

Thx to @ACE Console Repairs for the diagram

### Optional: 0.91" OLED Display (SSD1306)

Used dimensions: 0.91" 128x32 pixels, monochrome

![SSD 1306 module](./assets/ssd1306_module.jpg)

- Pi Pico 3V3 -> Display VCC
- SDA: Pi Pico **Pin  9** (GP6) -> Display **Pin SDA**
- SCL: Pi Pico **Pin 10** (GP7) -> Display **Pin SCL**
- GND -> GND

![OLED Display with POST code](./assets/display.jpg)

## Usage

- Download [latest release](https://github.com/xboxoneresearch/PicoDurangoPOST/releases/latest)
- Flash/Copy *.uf2 onto Pi Pico
- Listen on the exposed USB Serial interface (via PuTTy or similar UART monitor software)

How to interact ?

- On Startup, it directly goes into POST monitoring mode.
- If you hit "CTRL+C" you will be brought into the REPL-menu.
- Here you can set various options or scan for I2C devices on the bus.
- Check out the "help" command.

![Example output](./assets/screenshot2.png)

## Develop

- VS Code
- Extension: PlatformIO
- Upload and Monitor
