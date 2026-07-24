#pragma once

#include <Wire.h>
#include <U8g2lib.h>

#define CODEBUF_SZ 18

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

static inline const u8g2_cb_t *getInternalRotation(DisplayRotation rotation, bool mirrored) {
    if (rotation == DISPLAY_LANDSCAPE) {
        return mirrored ? U8G2_R2 : U8G2_R0;
    } else if (rotation == DISPLAY_PORTRAIT) {
        return mirrored ? U8G2_R3 : U8G2_R1;
    }

    return U8G2_R0;
}

class Display {
public:
    Display(uint8_t i2cAddress, uint8_t sdaPin, uint8_t sclPin, U8G2 displayInstance)
    {
        _sdaPin = sdaPin;
        _sclPin = sclPin;
        display = displayInstance;
        address = i2cAddress;
        currentRotation = DISPLAY_LANDSCAPE;
        // Up to 16 hex digits (uint64_t) + newline + NUL
        codeBuf = (char *) calloc(1, CODEBUF_SZ);
    }
    ~Display() {
        if (codeBuf != NULL) {
            free(codeBuf);
        }
    }
    bool begin();

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

        display.setDisplayRotation(getInternalRotation(rotation, mirrored));
        display.clearBuffer();
        cursorY = 0;

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

        display.clearBuffer();
        cursorY = 0;
    }

    void printMessage(const char *header, const char *text, int durationMs = 1000);
    void printCenteredH(const char *text, int16_t y);
    void printCode(uint64_t code, const char *flavor);
private:
    uint8_t address;
    uint8_t _sdaPin;
    uint8_t _sclPin;
    DisplayRotation currentRotation;
    U8G2 display;
    bool mirrored = false;
    bool initialized = false;
    int16_t cursorY = 0;
    char *codeBuf = NULL;
};
