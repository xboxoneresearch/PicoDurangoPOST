#include <cppQueue.h>
#include "common.h"

// Used to indicate I2C scan finish from core1->core0
#define I2C_SCAN_FINISHED 0xFF

#if true
// Only enqueue segment if segment's lower nibble > 0
#define SHOULD_ENQUEUE_SEGMENT(x) ((x & 0x0F) != 0)
#else
// Enqueue every segment
#define SHOULD_ENQUEUE_SEGMENT(x) (true)
#endif

// Message from core0->core1
bool showTimestamp = true;
uint32_t msg_core0 = 0;
uint64_t timestamp = time_us_64();
SegmentData currentSegData = {0};
bool postMonitorRunning = false;
State currentState = STATE_POST_MONITOR;
String inputBuffer = "";
uint8_t registers[MAX6958_REGISTER_SIZE];
char *codeName = (char*)calloc(1, 255);

// Create a queue for POST codes
cppQueue	postCodeQueue(sizeof(SegmentData), POST_MAX_QUEUE_SIZE, FIFO);
Display     display(DISP_SCREEN_WIDTH, DISP_SCREEN_HEIGHT, PIN_SDA_DISP, PIN_SCL_DISP, SSD1306_DISP_ADDRESS, &Wire1);

enum CrossThreadMsg: uint32_t {
    INVALID = 0,
    SCAN_I2C = 1,
    RESET_TIMESTAMP = 2
};

char *postCodeToName(uint16_t code) {
    uint8_t loByte = code & 0xFF;
    uint8_t hiByte = (code & 0xFF00) >> 8;
    snprintf(codeName, 255, "\0");

    if (code == 0x14FF || code == 0x0075) {
        // Exit early with generic names
        return (char *)"BOOT_SUCCESS";
    }
    
    // Match on range 0x01xx - 0x14xx (2BL)
    if (hiByte >= POST_2BL_STAGE_UNKNOWN && hiByte <= POST_2BL_FINAL) {
        // Search name for 2BL stage / hi byte
        const char *phase2BL = getNameFor2BlPhase(hiByte);
        // Search AMD AGESA / ABL PostCode / testpoint - lo byte
        const char *tp2BL = getNameForAblTestpoint(loByte);

        snprintf(codeName, 255, "%s_%s\0", phase2BL, tp2BL);
        return codeName;
    }

    // Match on distinct u16 codes
    char *res = getNameForPostcode(code);
    if (res) {
        return res;
    }

    switch (hiByte) {
        case POST_SMC_FATAL:
            snprintf(
                codeName, 255, "%s_%02x\0",
                "SMC_FATAL_UNKNOWN",
                loByte
            );
            break;
        case POST_SMC_THERMAL:
            snprintf(
                codeName, 255, "%s_%02x\0",
                "SMC_THERMAL_UNKNOWN",
                loByte
            );
            break;
        case POST_SMC_BOOT:
            snprintf(
                codeName, 255, "%s_%02x\0",
                "SMC_BOOT_UNKNOWN",
                loByte
            );
            break;
        case POST_SMC_RUNTIME:
            snprintf(
                codeName, 255, "%s_%02x\0",
                "SMC_RUNTIME_UNKNOWN",
                loByte
            );
            break;
        case POST_SP_UNKNOWN:
            snprintf(
                codeName, 255, "%s_%02x\0",
                "SP_UNKNOWN",
                loByte
            );
            break;
    }

    if (strlen(codeName) > 0) {
        return codeName;
    }

    return NULL;
}

uint16_t segmentDigitsToCode(SegmentData *segData) {
    uint16_t code = 0;
    code |= segData->digits[0] & 0x0F;
    code |= (segData->digits[1] & 0x0F) << 4;
    code |= (segData->digits[2] & 0x0F) << 8;
    code |= (segData->digits[3] & 0x0F) << 12;
    return code;
}

void printHelp() {
    Serial.println("\r\nAvailable commands:");
    Serial.println("  post    - Start POST code monitoring");
    Serial.println("  scan    - Scan for I2C devices");
    
    // Modifiers for POST monitor
    Serial.println("  ts      - Toggle showing timestamps");
    Serial.println("  rotate  - Rotate display");
    
    Serial.println("  help    - Show this help message");
    Serial.println("  CTRL+C  - Exit current mode and return to REPL");
}

void print(const char* header, const char *text, int durationMs = 0) {
    Serial.printf("%s: %s\r\n", header, text);
    display.printMessage(header, text, durationMs);
}

void handleRepl() {
    if (Serial.available()) {
        char c = Serial.read();
        
        // Echo character in REPL mode
        if (currentState == STATE_REPL && isalpha(c)) {
            Serial.write(c);
        }
        
        // Handle newline
        if (c == '\n' || c == '\r') {
            Serial.println("");
            if (inputBuffer.length() > 0) {
                inputBuffer.trim();
                if (inputBuffer == "post") {
                    currentState = STATE_POST_MONITOR;
                } else if (inputBuffer == "scan") {
                    currentState = STATE_I2C_SCAN;
                } else if (inputBuffer == "rotate") {
                    currentState = STATE_DISPLAY_ROTATE;
                } else if (inputBuffer == "ts") {
                    currentState = STATE_TOGGLE_TIMESTAMP;
                } else if (inputBuffer == "help") {
                    printHelp();
                } else {
                    Serial.println("Unknown command. Type 'help' for available commands.");
                }
                inputBuffer = "";
            }
            if (currentState == STATE_REPL) {
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
        int val = registers[i];
        Serial.printf("REG 0x%02X : 0x%02X\r\n", i, val);
    }
}

void printCode(uint16_t code, uint8_t segment, uint64_t timestamp) {
    char *name = postCodeToName(code);
    const char *flavor = getCodeFlavorForSegment(segment);
    // Lower nibble of segment == position of code when > u16?
    uint8_t segmentNibble = segment & 0x0F;

    display.printCode(code, flavor, name, segmentNibble);
    Serial.printf("%s (%i): 0x%04x",
        flavor,
        segmentNibble,
        code
    );

    if (name != NULL) {
        Serial.printf(" [%s] ", name);
    }

    if (showTimestamp) {
        Serial.print("(");
        Serial.print(timestamp / 1000.0);
        Serial.print(" ms)");
    }

    Serial.println();
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
            registers[reg] = Wire.read();
            // Serial.printf("Register %s (0x%02x): 0x%02x\r\n", getNameForMAX6958Register(reg), reg, registers[reg]);
            if (reg == Segments) {
                // Signal that we have a new POST code
                SegmentData segData = {0};
                // Get uS relative to last timer reset
                segData.timestamp = time_us_64() - timestamp;
                segData.segments  = registers[Segments];
                segData.digits[0] = registers[Digit0];
                segData.digits[1] = registers[Digit1];
                segData.digits[2] = registers[Digit2];
                segData.digits[3] = registers[Digit3];
                
                // Add code to queue if it's not full
                if (SHOULD_ENQUEUE_SEGMENT(segData.segments) && !postCodeQueue.isFull()) {
                    postCodeQueue.push((SegmentData *)&segData);
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
                timestamp = time_us_64();
                postCodeQueue.clean();
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
    if (display.setup()) {
        Serial.println("SSD1306 Display detected :)");
    } else {
        Serial.println("No display detected :(");
    }
    print("Firmware", __FW_VERSION__, 1000);
    
    Serial.println("POST Reader I2C");
}

void loop() {
    switch (currentState) {
        case STATE_RETURN_TO_REPL:
            if (postMonitorRunning) {
                postMonitorRunning = false;
            }

            currentState = STATE_REPL;
            Serial.println("Returning to REPL...");
            printHelp();
            Serial.print(">> ");  // REPL prompt after returning
            break;

        case STATE_REPL:
            handleRepl();
            break;
        
        case STATE_POST_MONITOR:
            if (!postMonitorRunning) {
                postMonitorRunning = true;
                sendMessageToCore1(RESET_TIMESTAMP);
                display.clear();
                Serial.println("Entering POST monitoring mode. Press CTRL+C to exit.");
            }

            if (Serial.peek() == 'r') {
                sendMessageToCore1(RESET_TIMESTAMP);
            }

            // Process all codes in the queue
            while (!postCodeQueue.isEmpty()) {
                if(postCodeQueue.pop((SegmentData*)&currentSegData)) {
                    uint16_t code = segmentDigitsToCode(&currentSegData);
                    printCode(code, currentSegData.segments, currentSegData.timestamp);
                }
            }
            break;
            
        case STATE_I2C_SCAN:
            scanForAvailableDevices();
            // After scan, jump right back into REPL
            currentState = STATE_RETURN_TO_REPL;
            break;
        case STATE_DISPLAY_ROTATE:
            display.toggleRotation();
            print("Notice", "Display rotated");
            currentState = STATE_RETURN_TO_REPL;
            break;
        case STATE_TOGGLE_TIMESTAMP:
            showTimestamp = !showTimestamp;
            print("Notice", "Toggled timestamps");
            currentState = STATE_RETURN_TO_REPL;
    }

    // Check for CTRL+C
    if (currentState != STATE_RETURN_TO_REPL
        && currentState != STATE_REPL
        && Serial.available()
        && Serial.read() == CTRL_C
    ) {
        currentState = STATE_RETURN_TO_REPL;
    }
}

/* CORE 0 END */