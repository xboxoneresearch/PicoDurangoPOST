
#include "common.h"

bool Display::begin(uint8_t sdaPin, uint8_t sclPin) {
    twoWirePort->setSDA(sdaPin);
    twoWirePort->setSCL(sclPin);
 
    twoWirePort->begin();
    twoWirePort->beginTransmission(address);
    auto err = twoWirePort->endTransmission();
    twoWirePort->end();

    if (err != 0) {
        // Device not reachable
        DBG("Display error: %i", err);
        return false;
    } else if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
        DBG("Failed to initialize / allocate data for display");
        return false;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    setRotation(currentRotation);

    initialized = true;
    return true;
}

void Display::printMessage(const char* header, const char *text, int durationMs) {
    if (!initialized) {
        return;
    }

    clear();
    setRotation(DISPLAY_LANDSCAPE);

    display.setTextSize(1);
    printCenteredH((char *)header);
    display.println();
    display.println();
    printCenteredH((char *)text);
    display.display();

    if (durationMs > 0) {
        sleep_ms(durationMs);
        display.clearDisplay();
    }

    setRotation(currentRotation);
}

void Display::printCentered(const char *text, int16_t x, int16_t y) {
    if (!initialized) {
        return;
    }

    int16_t x1, y1;
    uint16_t w, h;

    if (x == 0)
        x = display.getCursorX();
    if (y == 0)
        y = display.getCursorY();

    display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
    display.setCursor(
        (width - w) / 2,
        (height - h) / 2
    );
    display.println(text);
}

void Display::printCenteredH(char *text, int16_t x, int16_t y) {
    if (!initialized) {
        return;
    }

    int16_t x1, y1;
    uint16_t w, h;

    if (x == 0)
        x = display.getCursorX();
    if (y == 0)
        y = display.getCursorY();

    display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
    display.setCursor(
        (width - w) / 2,
        y / 2
    );
    display.println(text);
}

void Display::printCode(uint16_t code, const char *flavor, uint8_t segmentNibble) {
    if (!initialized) {
        return;
    }

    if (isDisplayPortrait() && display.getCursorY() >= width) {
        clear();
    }
    else if (isDisplayLandscape()) {
        // On portrait mode we want to keep older lines
        clear();
    }

    // Bigger size for landscape mode
    display.setTextSize(isDisplayLandscape() ? 3 : 1);
    sprintf(codeBuf, "%04X\n", code);

    if (isDisplayLandscape()) {
        printCenteredH(codeBuf);
    } else {
        display.print(codeBuf);
    }

    if (isDisplayLandscape()) {
        display.setTextSize(1);

        // Print Code flavor in the top-left corner
        display.setCursor(0,0);
        display.printf("%s\n(%i)", flavor, segmentNibble);

        display.setTextSize(2);
    }
    display.display();
}