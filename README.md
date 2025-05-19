[![PlatformIO CI](https://github.com/xboxoneresearch/PicoDurangoPOST/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/xboxoneresearch/PicoDurangoPOST/actions/workflows/build.yml)
![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/xboxoneresearch/PicoDurangoPOST/latest/total)
[![GitHub latest Tag](https://img.shields.io/github/v/tag/xboxoneresearch/PicoDurangoPOST)](https://github.com/xboxoneresearch/PicoDurangoPOST/releases/latest)

# Durango POST-Code reader via RasPi Pico

This software flashed to the Raspberry Pi Pico allows for monitoring of POST codes, which is useful for fault diagnosis on the hardware.

Jump to [Usage](#usage)

Technical infos:

- [POST bus on Xbox One - Wiki page](https://xboxoneresearch.github.io/wiki/hardware/post/)

The wiki page contains a list of POST codes that are already known. 

Also check out the video from @craftbenmine

[![Watch the video](https://img.youtube.com/vi/BuPhxKdxU0g/hqdefault.jpg)](https://www.youtube.com/embed/BuPhxKdxU0g)

## Connections

> [!IMPORTANT]
> With version v0.1.3, pin assignments for I2C got changed to GP0/GP1, to be in-sync with the pins used by [DuRFUnitI2C](https://github.com/xboxoneresearch/DuRFUnitI2C).
> Before that I2C was on GP4/GP5.

Pi Pico -> FACET

- SDA: Pi Pico Pin 0 (GP0) -> FACET Pin 26
- SCL: Pi Pico Pin 1 (GP1) -> FACET Pin 25
- GND -> GND

![Pi Pico Facet I2C connection diagram](./assets/connection_diagram.png)

### Optional: 0.91" OLED Display (SSD1306)

- Pi Pico 3V3 -> Display VCC
- SDA: Pi Pico Pin  9 (GP6) -> Display Pin SDA
- SCL: Pi Pico Pin 10 (GP7) -> Display Pin SCL
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
