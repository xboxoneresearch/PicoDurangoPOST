
#include "display.h"
#include "common.h"

#define FONT_SMALL u8g2_font_6x10_tf
#define FONT_LARGE u8g2_font_profont22_tr

bool Display::begin() {
    // U8g2's "2ND_HW_I2C" HAL is hardwired to talk to Wire1 and only ever
    // calls Wire1.begin() with no pin args, so custom pins must be staged
    // on Wire1 before display.begin() hands off to that HAL.
#if defined(ARDUINO_ARCH_RP2040)
    Wire1.setSDA(_sdaPin);
    Wire1.setSCL(_sclPin);
    Wire1.begin();
#elif defined(ARDUINO_ARCH_ESP32)
    Wire1.begin(sdaPin, sclPin);
#elif defined(TEENSYDUINO)
    Wire1.begin(); // pins fixed in hardware, not configurable
#endif

    display.setI2CAddress(address << 1);
    display.begin();

    setRotation(currentRotation);

    initialized = true;
    return true;
}

void Display::printMessage(const char* header, const char *text, int durationMs) {
    if (!initialized) {
        return;
    }

    DisplayRotation prevRotation = currentRotation;

    clear();
    setRotation(DISPLAY_LANDSCAPE);

    display.setFont(FONT_SMALL);
    printCenteredH(header, 12);
    printCenteredH(text, 28);
    display.sendBuffer();

    if (durationMs > 0) {
        delay(durationMs);
        clear();
    }

    setRotation(prevRotation);
}

void Display::printCenteredH(const char *text, int16_t y) {
    if (!initialized) {
        return;
    }

    int16_t w = display.getStrWidth(text);
    display.drawStr((display.getDisplayWidth() - w) / 2, y, text);
}

void Display::printCode(uint64_t code, const char *flavor) {
    if (!initialized) {
        return;
    }

    if (isDisplayPortrait() && cursorY >= display.getDisplayWidth()) {
        clear();
    }
    else if (isDisplayLandscape()) {
        // On portrait mode we want to keep older lines
        clear();
    }

    snprintf(codeBuf, CODEBUF_SZ, "%04llX", (unsigned long long)code);

    if (isDisplayLandscape()) {
        display.setFont(FONT_LARGE);
        printCenteredH(codeBuf, display.getDisplayHeight() - 4);

        // Print Code flavor in the top-left corner
        display.setFont(FONT_SMALL);
        display.drawStr(0, 8, flavor);
    } else {
        display.setFont(FONT_SMALL);
        cursorY += display.getMaxCharHeight() + 1;
        display.drawStr(0, cursorY, codeBuf);
    }

    display.sendBuffer();
}
