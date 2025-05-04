#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

enum DisplayRotation {
    DISP_ROTATION_LANDSCAPE = 0,
    DISP_ROTATION_PORTRAIT = 1,
    DISP_ROTATION_LANDSCAPE_ROTATED = 2,
    DISP_ROTATION_PORTRAIT_ROTATED = 3,
};

class Display {
public:
    Display(
        uint8_t displayWidth,
        uint8_t displayHeight,
        uint8_t sdaPin,
        uint8_t sclPin,
        uint8_t i2cAddress,
        TwoWire *wire
    );
    ~Display();
    bool setup();
    DisplayRotation getCurrentRotation() {
        return currentRotation;
    }
    
    bool isDisplayLandscape() {
        return (
            currentRotation == DISP_ROTATION_LANDSCAPE
            || currentRotation == DISP_ROTATION_LANDSCAPE_ROTATED
        );
    }
    
    bool isDisplayPortrait() {
        return (
            currentRotation == DISP_ROTATION_PORTRAIT
            || currentRotation == DISP_ROTATION_PORTRAIT_ROTATED
        );
    }

    void clear() {
        display.setCursor(0, 0);
        display.clearDisplay();
    }

    void printMessage(const char *header, const char *text, int durationMs = 1000);
    void printCentered(const char *text, int16_t x = 0, int16_t y = 0);
    void printCenteredH(char *text, int16_t x = 0, int16_t y = 0);
    void updateDisplayRotation(DisplayRotation rotation);
    void printCode(uint16_t code, const char *name);
private:
    TwoWire *twoWirePort;
    uint8_t width;
    uint8_t height;
    uint8_t sda;
    uint8_t scl;
    uint8_t address;
    DisplayRotation currentRotation;
    Adafruit_SSD1306 display;
    bool initialized = false;
};