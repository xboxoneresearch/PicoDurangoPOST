
#include "display.h"

char *codeBuf = (char *) calloc(5, 0);

Display::Display(uint8_t displayWidth, uint8_t displayHeight, uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress, TwoWire *wire) {
    twoWirePort = wire;
    sda = sdaPin;
    scl = sclPin;
    address = i2cAddress;
    width = displayWidth;
    height = displayHeight;
    currentRotation = DISP_ROTATION_LANDSCAPE;
    display = Adafruit_SSD1306(width, height, twoWirePort, -1);
}

bool Display::setup() {
    Wire1.setSDA(sda);
    Wire1.setSCL(scl);

    if (display.begin(SSD1306_SWITCHCAPVCC, address))
    {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setRotation(currentRotation);

        initialized = true;
    }

    return initialized;
}

void Display::printMessage(const char* header, const char *text, int durationMs) {
    if (!initialized) {
        return;
    }

    clear();
    display.setRotation(DISP_ROTATION_LANDSCAPE);

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

    display.setRotation(currentRotation);
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

void Display::updateDisplayRotation(DisplayRotation rotation) {
    if (!initialized) {
        return;
    }

    display.setRotation(DISP_ROTATION_LANDSCAPE);
    display.clearDisplay();
    display.setTextSize(1);
    printCentered("Updated rotation");
    display.display();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setRotation(rotation);

    currentRotation = rotation;
}

void Display::printCode(uint16_t code, const char *name) {
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
        // In landscape orientation, also print POST-code name
        if (name != NULL) {
            printCenteredH((char *)name);
        } else {
            printCenteredH((char *)"UNKNOWN");
        }
        display.setTextSize(2);
    }
    display.display();
}