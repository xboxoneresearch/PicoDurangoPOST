#include "common.h"
#include "colors.h"
#include "codes.h"
#include "config.h"

// Used to indicate I2C scan finish from core1->core0
#define I2C_SCAN_FINISHED 0xFF

#if true
// Only enqueue segment if segment's lower nibble > 0
#define SHOULD_ENQUEUE_SEGMENT(x) ((x & 0x0F) != 0)
#else
// Enqueue every segment
#define SHOULD_ENQUEUE_SEGMENT(x) (true)
#endif

#ifndef __FW_VERSION__
#define FW_VERSION "unknown version"
#else
#define FW_VERSION __FW_VERSION__
#endif

#ifndef __BUILD_DATE__
#define BUILD_DATE 0
#else
#define BUILD_DATE __BUILD_DATE__
#endif


#define PRINT_COLOR(c, x) \
    if (cfg.isSerialPrintColors()) \
        Serial.print(c); \
    x; \
    if (cfg.isSerialPrintColors()) \
        Serial.print(COLOR_RESET);


// Message from core0->core1
uint32_t msg_core0 = 0;

SegmentData currentSegData = {0};
bool postMonitorRunning = false;

String inputBuffer = "";

Config cfg;
RuntimeState runtimeState;

bool postCodeToName(uint8_t segment, uint16_t code, char *outName, bool *isErrorCode) {
    bool ret = false;
    CodeFlavor flavor = (CodeFlavor)(segment & 0xF0);

    switch (flavor) {
        case CODE_FLAVOR_SMC:
            ret = getNameForSmcCode(segment, code, outName, isErrorCode);
            break;
        case CODE_FLAVOR_SP:
            ret = getNameForSpCode(segment, code, outName, isErrorCode);
            break;
        case CODE_FLAVOR_CPU:
            ret = getNameForCpuCode(segment, code, outName, isErrorCode);
            break;
        case CODE_FLAVOR_OS:
            ret = getNameForOsCode(segment, code, outName, isErrorCode);
            break;
    }

    return ret;
}

uint16_t segmentDigitsToCode(SegmentData *segData) {
    uint16_t code = 0;
    code |= segData->digits[0] & 0x0F;
    code |= (segData->digits[1] & 0x0F) << 4;
    code |= (segData->digits[2] & 0x0F) << 8;
    code |= (segData->digits[3] & 0x0F) << 12;
    return code;
}


void print(const char* header, const char *text, int durationMs = 0) {
    Serial.printf("%s: %s\r\n", header, text);
    runtimeState.display()->printMessage(header, text, durationMs);
}

void printFwVersion(bool startup = false) {
    char *fwString = (char *)calloc(1, 255);
    snprintf(fwString, 255, "%s %lu", FW_VERSION, BUILD_DATE);
    // Only delay the print on startup
    print("FW", fwString, startup ? 2000 : 0);
    free(fwString);
    if (startup) {
        runtimeState.display()->printMessage("Presented by", "xboxresearch.com");
    }
}

void printHelp() {
    Serial.println("PicoDurangoPOST - by XboxOneResearch\r\n");
    Serial.println("Website: https://xboxresearch.com");
    Serial.println("Source: https://github.com/XboxOneResearch/PicoDurangoPOST");
    Serial.println("\r\nAvailable commands:");
    Serial.println("  post    - Start POST code monitoring");
    Serial.println("  scan    - Scan for I2C devices");
    // Modifiers for POST monitor
    Serial.println("\r\nPOST modifiers:");
    Serial.println("  ts      - Toggle showing timestamps");
    Serial.println("  colors  - Print colors over serial");
    Serial.println("  rotate  - Rotate display");
    Serial.println("  mirror  - Mirror display");
    Serial.println("\r\nConfig:");
    Serial.println("  config  - Show config");
    Serial.println("  save    - Save config");
    Serial.println("\r\nGeneral:");
    Serial.println("  version - Show firmware version");
    Serial.println("  help    - Show this help message");
    Serial.println("  CTRL+C  - Exit current mode and return to REPL");
}

void handleRepl() {
    if (Serial.available()) {
        char c = Serial.read();
        
        // Echo character in REPL mode
        if (runtimeState.getCurrentState() == STATE_REPL && isalpha(c)) {
            Serial.write(c);
        }
        
        // Handle newline
        if (c == '\n' || c == '\r') {
            Serial.println("");
            if (inputBuffer.length() > 0) {
                inputBuffer.trim();
                if (inputBuffer == "post") {
                    runtimeState.setCurrentState(STATE_POST_MONITOR);
                } else if (inputBuffer == "scan") {
                    runtimeState.setCurrentState(STATE_I2C_SCAN);
                } else if (inputBuffer == "rotate") {
                    runtimeState.setCurrentState(STATE_DISPLAY_ROTATE);
                } else if (inputBuffer == "mirror") {
                    runtimeState.setCurrentState(STATE_DISPLAY_MIRROR);
                } else if (inputBuffer == "colors") {
                    runtimeState.setCurrentState(STATE_TOGGLE_COLORS);
                } else if (inputBuffer == "ts") {
                    runtimeState.setCurrentState(STATE_TOGGLE_TIMESTAMP);
                } else if (inputBuffer == "config") {
                    runtimeState.setCurrentState(STATE_CONFIG_SHOW);
                } else if (inputBuffer == "save") {
                    runtimeState.setCurrentState(STATE_CONFIG_SAVE);
                } else if (inputBuffer == "version" ) {
                    runtimeState.setCurrentState(STATE_PRINT_VERSION);
                } else if (inputBuffer == "help") {
                    runtimeState.setCurrentState(STATE_PRINT_HELP);
                } else {
                    Serial.println("Unknown command. Type 'help' for available commands.");
                }
                inputBuffer = "";
            }
            if (runtimeState.getCurrentState() == STATE_REPL) {
                Serial.print(">> ");  // REPL prompt after command
            }
        } else if (isalpha(c)) {
            inputBuffer += c;
        }
    }
}

void printRegisters() {
    Serial.println(">> REGISTERS");
    for (int i = 0; i < MAX6958_REGISTER_SIZE; i++) {
        int val = runtimeState.getRegister(i);
        Serial.printf("REG 0x%02X : 0x%02X\r\n", i, val);
    }
}

void printCode(uint16_t code, uint8_t segment, uint64_t timestamp) {
    bool isErrorCode = false;
    char *codeName = (char*)calloc(1, 255);

    bool success = postCodeToName(segment, code, codeName, &isErrorCode);
    const char *flavor = getCodeFlavorForSegment(segment);
    // Lower nibble of segment == position of code when > u16?
    uint8_t segmentNibble = segment & 0x0F;

    runtimeState.display()->printCode(code, flavor, success ? codeName: NULL, segmentNibble);
    
    // Color is only printed if `printColors` is set
    PRINT_COLOR(COLOR_FLAVOR, Serial.print(flavor))
    Serial.print(" (");
    PRINT_COLOR(COLOR_SEG_INDEX, Serial.print(segmentNibble));
    Serial.print("): ");

    if (isErrorCode) {
        PRINT_COLOR(COLOR_ERROR, Serial.printf("0x%04x", code))
    } else {
        PRINT_COLOR(COLOR_CODE, Serial.printf("0x%04x", code))
    }

    if (success && strlen(codeName) > 0) {
        Serial.print(" [");
        if (isErrorCode) {
            PRINT_COLOR(COLOR_ERROR, Serial.print(codeName))
        } else {
            PRINT_COLOR(COLOR_NAME, Serial.print(codeName))
        }
        Serial.print("]");
    }

    if (cfg.isPostPrintTimestamps()) {
        Serial.print(" (");
        PRINT_COLOR(COLOR_TIMESTAMP, Serial.printf("%.0f", timestamp / 1000.0))
        Serial.print(" ms");
        Serial.print(")");
    }

    Serial.println();

    free(codeName);
}

/* CORE 1 START */

void core1_receiveI2cData(int howMany) {
    int reg = -1;
    while (Wire.available()) {
        if (reg == -1 || reg == FactoryReserved || reg == MAX6958_REGISTER_SIZE) {
            // First byte of a packet is the CMD / target register address
            // NOTE: Register Configuration (0x04) is always sent alone
            // If we are at Register FactoryReserved (0x05), we read the register/command again, as it will be the start of a new packet
            // If Register is > MAX6958_REGISTER_SIZE, also expect a new packet following
            reg = Wire.read();
        } else {
            runtimeState.setRegister(reg, Wire.read());
            // Serial.printf("Register %s (0x%02x): 0x%02x\r\n", getNameForMAX6958Register(reg), reg, registers[reg]);
            if (reg == Segments) {
                // Signal that we have a new POST code
                SegmentData segData = {0};
                // Get uS relative to last timer reset
                segData.timestamp = time_us_64() - runtimeState.getTimestamp();
                segData.segments  = runtimeState.getRegister(Segments);
                segData.digits[0] = runtimeState.getRegister(Digit0);
                segData.digits[1] = runtimeState.getRegister(Digit1);
                segData.digits[2] = runtimeState.getRegister(Digit2);
                segData.digits[3] = runtimeState.getRegister(Digit3);
                
                // Add code to queue if it's not full
                if (SHOULD_ENQUEUE_SEGMENT(segData.segments) && !runtimeState.isPostCodeQueueFull()) {
                    runtimeState.pushPostCode((SegmentData *)&segData);
                }
            }
            // After each byte the target register address is automatically incremented
            // This allow communication to be more dense
            // See: MAX6958 Datasheet, page 9 -> Command Address Autoincrementing
            reg++;
        }
    }
}

void core1_i2cScan(TwoWire *interface) {
    interface->begin();

    // As I2C Master, scan for available devices on the bus
    for (uint8_t addr = 1; addr < 0x7F; addr++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device acknowledged the transfer to the address.
        interface->beginTransmission(addr);
        // Check if transmission succeeded
        // If true, device @ address is available
        if (interface->endTransmission() == 0) {
            rp2040.fifo.push(addr);
        }
    }
    // Signal that scan is finished
    rp2040.fifo.push(I2C_SCAN_FINISHED);

    interface->end();
}

void setup1() {
    Wire.setSDA(PIN_SDA_XBOX);
    Wire.setSCL(PIN_SCL_XBOX);

    Wire.begin(MAX6958_ADDRESS);
    Wire.onReceive(core1_receiveI2cData);
}

void loop1() {
    // Just handle I2C interrupts in the background or react to message from core0 to do I2C Scan
    if (rp2040.fifo.available() && rp2040.fifo.pop_nb(&msg_core0)) {
        switch (msg_core0) {
            case SCAN_I2C:
                // Stop I2C slave
                Wire.end();

                // Do I2C scan as master
                core1_i2cScan(&Wire);

                // Start slave operation again
                Wire.begin(MAX6958_ADDRESS);
                Wire.onReceive(core1_receiveI2cData);
                break;
            case RESET_TIMESTAMP:
                runtimeState.resetTimestamp();
                runtimeState.clearPostCodeQueue();
                break;
        }
    }
}

/* CORE 1 START */

/* CORE 0 START */

inline void sendMessageToCore1(CrossThreadMsg msg) {
    rp2040.fifo.push_nb(msg);
}

void scanForAvailableDevices() {
    int error = 0;
    int nDevices = 0;
    // Instruct core1 to scan for available devices on the bus
    Serial.println("Scanning for I2C devices...");
    sendMessageToCore1(SCAN_I2C);
    sleep_ms(1000);

    uint32_t address = 0;
    // Get enumerated addresses from core1 until "SCAN_FINISHED" is signaled
    while (rp2040.fifo.pop_nb(&address) && address != I2C_SCAN_FINISHED) {
        uint8_t addr_u8 = address & 0xFF;
        const char *deviceName = getDeviceNameForI2cAddress(addr_u8);

        Serial.printf("- Address: %i (hex: 0x%02x) [%s]\r\n", addr_u8, addr_u8, deviceName);
        nDevices++;
    }

    if (nDevices > 0)
        Serial.printf("Found %d devices on I2C Bus\r\n", nDevices);
    else
        Serial.println("No devices found on I2C bus...");
}


void setup() {
#if WAIT_FOR_SERIAL
    // Wait for serial to be connected
    // before continuing with setup
    while (!Serial) {
        delay(10);
    }
#endif
    Serial.begin(SERIAL_BAUD);
    if (!cfg.begin()) {
        Serial.println("Failed to load config");
    }

    if (runtimeState.begin()) {
        Serial.println("SSD1306 Display detected :)");
        // Set display rotation
        runtimeState.display()->setRotation(
            cfg.isRotationPortrait()
            ? DISPLAY_PORTRAIT
            : DISPLAY_LANDSCAPE
        );
        runtimeState.display()->setMirroring(cfg.isDisplayMirrored());
    } else {
        Serial.println("No display detected :(");
    }

    printFwVersion(true);
    Serial.println("POST Reader I2C");
}

void loop() {
    switch (runtimeState.getCurrentState()) {
        case STATE_RETURN_TO_REPL:
            if (postMonitorRunning) {
                postMonitorRunning = false;
            }

            runtimeState.setCurrentState(STATE_REPL);
            Serial.print(">> ");  // REPL prompt after returning
            break;

        case STATE_REPL:
            handleRepl();
            break;
        
        case STATE_POST_MONITOR:
            if (!postMonitorRunning) {
                postMonitorRunning = true;
                sendMessageToCore1(RESET_TIMESTAMP);
                runtimeState.display()->clear();
                Serial.println("Entering POST monitoring mode. Press CTRL+C to exit.");
            }

            // Process all codes in the queue
            while (!runtimeState.isPostCodeQueueEmpty()) {
                if(runtimeState.popPostCode(&currentSegData)) {
                    uint16_t code = segmentDigitsToCode(&currentSegData);
                    printCode(code, currentSegData.segments, currentSegData.timestamp);
                }
            }
            break;
            
        case STATE_I2C_SCAN:
            scanForAvailableDevices();
            // After scan, jump right back into REPL
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_DISPLAY_ROTATE:
            runtimeState.display()->toggleRotation();
            cfg.toggleRotationPortrait();
            print("Notice", "Display rotated");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_DISPLAY_MIRROR:
            runtimeState.display()->toggleMirroring();
            cfg.toggleDisplayMirrored();
            print("Notice", "Display mirrored");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_TOGGLE_TIMESTAMP:
            cfg.togglePostPrintTimestamps();
            print("Notice", "Toggled timestamps");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_TOGGLE_COLORS:
            cfg.toggleSerialPrintColors();
            print("Notice", "Toggled printing colors");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_CONFIG_SHOW:
            print("Notice", "Showing config");
            Serial.printf("Display mirrored:       %s\r\n", cfg.isDisplayMirrored() ? "ON" : "OFF");
            Serial.printf("Disp rotation portrait: %s\r\n", cfg.isRotationPortrait() ? "ON" : "OFF");
            Serial.printf("Print timestamps:       %s\r\n", cfg.isPostPrintTimestamps() ? "ON" : "OFF");
            Serial.printf("Print colors:           %s\r\n", cfg.isSerialPrintColors() ? "ON" : "OFF");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_CONFIG_SAVE:
            cfg.save();
            print("Notice", "Saved config");
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_PRINT_VERSION:
            printFwVersion();
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
        case STATE_PRINT_HELP:
            printHelp();
            runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
            break;
    }

    // Check for CTRL+C
    if (runtimeState.getCurrentState() != STATE_RETURN_TO_REPL
        && runtimeState.getCurrentState() != STATE_REPL
        && Serial.available())
    {
        int ch = Serial.read();

        switch (ch) {
            case CTRL_C:
                runtimeState.setCurrentState(STATE_RETURN_TO_REPL);
                break;
            case 'r':
                sendMessageToCore1(RESET_TIMESTAMP);
                Serial.println("Resetting timestamp");
                break;
        }
    }
}

/* CORE 0 END */