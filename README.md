# Durango POST-Code reader via RasPi Pico

This software for the Raspberry Pi Pico serves two purposes.

- On Startup, it does a I2C bus scan, to enumerate available devices (f.e. Voltage control ICs)
- After that, it goes into I2C Slave Mode and poses as MAX6958A (7-Segment-Display-IC). This allows to listen for POST codes.

Technical infos:

- [POST bus on Xbox One](https://xboxoneresearch.github.io/wiki/hardware/post/)

Interpretation of the values? That's not figured out yet lol

## Connections

Pi Pico -> FACET

SDA: Pi Pico Pin 6 (GP4) -> FACET Pin 26
SCL: Pi Pico Pin 7 (GP5) -> FACET Pin 25
GND -> GND

## Usage

- Flash/Copy *.uf2 onto Pi Pico
- Listen on the exposed USB Serial interface (via PuTTy or similar UART monitor software)

![Example output](./assets/screenshot.png)

## Develop

- VS Code
- Extension: PlatformIO
- Upload and Monitor