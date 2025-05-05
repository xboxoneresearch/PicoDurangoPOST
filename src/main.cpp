#include <cppQueue.h>
#include "common.h"
#include "display.h"

uint16_t code = 0;
bool postMonitorRunning = false;
State currentState = STATE_POST_MONITOR;
String inputBuffer = "";
uint8_t registers[MAX6958_REGISTER_SIZE];

// Create a queue for POST codes
cppQueue	postCodeQueue(sizeof(uint16_t), POST_MAX_QUEUE_SIZE, FIFO);
Display     display(DISP_SCREEN_WIDTH, DISP_SCREEN_HEIGHT, PIN_SDA_DISP, PIN_SCL_DISP, SSD1306_DISP_ADDRESS, &Wire1);

void receiveEvent(int howMany) {
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
                uint16_t code = 0;
                code |= registers[Digit0] & 0x0F;
                code |= (registers[Digit1] & 0x0F) << 4;
                code |= (registers[Digit2] & 0x0F) << 8;
                code |= (registers[Digit3] & 0x0F) << 12;
                
                // Add code to queue if it's not full
                if (!postCodeQueue.isFull()) {
                    postCodeQueue.push((uint16_t *)&code);
                }
            }
            // After each byte the target register address is automatically incremented
            // This allow communication to be more dense
            // See: MAX6958 Datasheet, page 9 -> Command Address Autoincrementing
            reg++;
        }
    }
}

void scanForAvailableDevices() {
    Wire.begin();
    int error = 0;
    int nDevices = 0;
    // As I2C Master, scan for available devices on the bus
    Serial.println("Scanning for I2C devices...");
    for (uint8_t addr = 1; addr < 0x7F; addr++) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(addr);
        error = Wire.endTransmission();

        const char *deviceName = getDeviceNameForI2cAddress(addr);
        if (error == 0)
        {
            Serial.printf("- Address: %i (hex: 0x%02x) [%s]\r\n", addr, addr, deviceName);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.printf("! Unknown error at address 0x%02x [%s]\r\n", addr, deviceName);
        }
    }
    if (nDevices > 0)
        Serial.printf("Found %d devices on I2C Bus\r\n", nDevices);
    else
        Serial.println("No devices found on I2C bus...");

    Wire.end();
}

void setupMax6958ASlave() {
    // As I2C Slave, simulate MAX6958
    Serial.println("POST Monitor (I2C Slave as MAX6958A) started");
    Wire.begin(MAX6958_ADDRESS);
    Wire.onReceive(receiveEvent);
    Serial.printf("Slave Address: 0x%02x\r\n", MAX6958_ADDRESS);
}

void printHelp() {
    Serial.println("\r\nAvailable commands:");
    Serial.println("  post    - Start POST code monitoring");
    Serial.println("  scan    - Scan for I2C devices");
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

void printCode(uint16_t code) {
    const char *name = getNameForPostcode(code);

    display.printCode(code, name);
    Serial.printf("CODE (SEG: 0x%02x): 0x%04x",
        registers[Segments],
        code
    );

    if (name != NULL) {
        Serial.printf(" [%s]", name);
    }

    Serial.println();
}

void setup() {
    // Wait for serial to be ready
    while (!Serial) {
        delay(10);
    }
    Serial.begin(115200);
    Wire.setSDA(PIN_SDA_XBOX);
    Wire.setSCL(PIN_SCL_XBOX);

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
            // Shutdown I2C Slave, in case we were in POST monitoring before
            if (postMonitorRunning) {
                postMonitorRunning = false;
                Wire.end();
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
                display.clear();
                setupMax6958ASlave();
                Serial.println("Entering POST monitoring mode. Press CTRL+C to exit.");
            }

            // Process all codes in the queue
            while (!postCodeQueue.isEmpty()) {
                if(postCodeQueue.pop((uint16_t*)&code))
                    printCode(code);
            }
            break;
            
        case STATE_I2C_SCAN:
            scanForAvailableDevices();
            // After scan, jump right back into REPL
            currentState = STATE_RETURN_TO_REPL;
            break;
        case STATE_DISPLAY_ROTATE:
            display.updateDisplayRotation(
                display.isDisplayLandscape()
                ? DISP_ROTATION_PORTRAIT
                : DISP_ROTATION_LANDSCAPE
            );
            print("Notice", "Display rotated");
            currentState = STATE_RETURN_TO_REPL;
            break;
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