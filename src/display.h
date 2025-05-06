#pragma once

#include <Adafruit_SSD1306.h>

enum DisplayRotation {
#if !MIRROR_DISPLAY_ROTATION
    DISP_ROTATION_LANDSCAPE = 0,
    DISP_ROTATION_PORTRAIT = 1,
#else
    DISP_ROTATION_LANDSCAPE = 2,
    DISP_ROTATION_PORTRAIT = 3,
#endif
};

class Display {
public:
    Display(uint8_t displayWidth, uint8_t displayHeight, uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress, TwoWire *wire) {
        twoWirePort = wire;
        sda = sdaPin;
        scl = sclPin;
        address = i2cAddress;
        width = displayWidth;
        height = displayHeight;
        currentRotation = DISP_ROTATION_LANDSCAPE;
        display = Adafruit_SSD1306(width, height, twoWirePort, -1);
        codeBuf = (char *) calloc(5, 0);
    }
    ~Display() {
        if (codeBuf != NULL) {
            free(codeBuf);
        }
    }
    bool setup();

    DisplayRotation getCurrentRotation() {
        return currentRotation;
    }
    
    bool isDisplayLandscape() {
        return (currentRotation == DISP_ROTATION_LANDSCAPE);
    }
    
    bool isDisplayPortrait() {
        return (currentRotation == DISP_ROTATION_PORTRAIT);
    }

    void toggleRotation() {
        updateDisplayRotation(
            isDisplayLandscape()
            ? DISP_ROTATION_PORTRAIT
            : DISP_ROTATION_LANDSCAPE
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
    void printCode(uint16_t code, uint8_t segment, const char *name);
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
    char *codeBuf = NULL;
};
