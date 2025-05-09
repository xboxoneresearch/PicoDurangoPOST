#pragma once

#include <Adafruit_SSD1306.h>

enum DisplayRotation {
    DISPLAY_LANDSCAPE = 0,
    DISPLAY_PORTRAIT = 1, 
};

// Internal enum only used in `getInternalRotation` / `Display::setRotation` !
enum INTERNAL_DisplayRotation {
    INTERNAL_DISP_ROTATION_LANDSCAPE = 0,
    INTERNAL_DISP_ROTATION_PORTRAIT = 1,
    INTERNAL_DISP_ROTATION_LANDSCAPE_MIRRORED = 2,
    INTERNAL_DISP_ROTATION_PORTRAIT_MIRRORED = 3,
};

static inline INTERNAL_DisplayRotation getInternalRotation(DisplayRotation rotation, bool mirrored) {
    if (rotation == DISPLAY_LANDSCAPE) {
        return mirrored ? INTERNAL_DISP_ROTATION_LANDSCAPE_MIRRORED : INTERNAL_DISP_ROTATION_LANDSCAPE;
    } else if (rotation == DISPLAY_PORTRAIT) {
        return mirrored ? INTERNAL_DISP_ROTATION_PORTRAIT_MIRRORED : INTERNAL_DISP_ROTATION_PORTRAIT;
    }

    return INTERNAL_DISP_ROTATION_LANDSCAPE;
}

class Display {
public:
    Display(uint8_t displayWidth, uint8_t displayHeight, uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress, TwoWire *wire) {
        twoWirePort = wire;
        sda = sdaPin;
        scl = sclPin;
        address = i2cAddress;
        width = displayWidth;
        height = displayHeight;
        currentRotation = DISPLAY_LANDSCAPE;
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
        return (currentRotation == DISPLAY_LANDSCAPE);
    }
    
    bool isDisplayPortrait() {
        return (currentRotation == DISPLAY_PORTRAIT);
    }

    void setRotation(DisplayRotation rotation) {
        if (!initialized) {
            return;
        }

        INTERNAL_DisplayRotation internalDisplayRotation = getInternalRotation(rotation, mirrored);
    
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setRotation(internalDisplayRotation);
    
        currentRotation = rotation;
    }

    void toggleRotation() {
        setRotation(
            isDisplayLandscape()
            ? DISPLAY_PORTRAIT
            : DISPLAY_LANDSCAPE
        );
    }

    void setMirroring(bool mirrorDisplay) {
        mirrored = mirrorDisplay;
        setRotation(currentRotation);
    }

    void toggleMirroring() {
        setMirroring(!mirrored);
    }

    void clear() {
        if (!initialized) {
            return;
        }

        display.setCursor(0, 0);
        display.clearDisplay();
    }

    void printMessage(const char *header, const char *text, int durationMs = 1000);
    void printCentered(const char *text, int16_t x = 0, int16_t y = 0);
    void printCenteredH(char *text, int16_t x = 0, int16_t y = 0);
    void printCode(uint16_t code, const char *flavor, const char *name, uint8_t segmentNibble);
private:
    TwoWire *twoWirePort;
    uint8_t width;
    uint8_t height;
    uint8_t sda;
    uint8_t scl;
    uint8_t address;
    DisplayRotation currentRotation;
    Adafruit_SSD1306 display;
    bool mirrored = false;
    bool initialized = false;
    char *codeBuf = NULL;
};
