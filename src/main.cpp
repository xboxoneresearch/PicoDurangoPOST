#include <Arduino.h>
#include <Wire.h>

// If you need to use different I/O pins for I2C, change it here
#define PIN_SDA 0
#define PIN_SCL 1
// ---

#define SLAVE_ADDRESS 0x38
#define REGISTER_SIZE 0x25

#define CTRL_C 3

// State machine states
enum State {
    STATE_RETURN_TO_REPL,
    STATE_REPL,
    STATE_POST_MONITOR,
    STATE_I2C_SCAN
};

State currentState = STATE_REPL;
String inputBuffer = "";

typedef struct {
    uint16_t code;
    const char *name;
} PostCode, *PPostCode;

typedef struct {
    uint8_t address;
    const char *name;
} I2cDevice, *PI2cDevice;

I2cDevice KnownDevices[] = {
    {0x15, "V_NBCORE"},
    {0x16, "V5_P0"},
    {0x18, "V_MEMIOAB"},
    {0x19, "V_MEMIOCD"},
    {0x20, "V_GFXCORE / V_CPUCORE"},
    {0x21, "V_MEMPHY/V_MEMIO/V_SOC"},
    {0x24, "GPIO EXPANDER (UNUSED)"},

    {0x2D, "V_DRAM1P8 DEBUG (UNUSED)"},
    {0x2E, "V_SOC1P8 DEBUG (UNUSED)"},
    {0x2F, "V_SOCPHY DEBUG (UNUSED)"},

    {0x40, "RAMVPP MONITOR (UNUSED)"},
    {0x41, "M2 MONITOR (UNUSED)"},
    {0x42, "CFEXPRESS MONITOR (UNUSED)"},
    {0x44, "12P0 MONITOR (UNUSED)"},
    {0x45, "12P0_SB MONITOR (UNUSED)"},
    {0x4A, "V_SOC1P8 MONITOR (UNUSED)"},
    {0x4B, "V_SOCPHY MONITOR (UNUSED)"},

    {0x38, "MAX6958/9A"},
    {0x39, "MAX6958/9B"},
    
    {0x5A, "RF Unit (ISD9160)"},
    {NULL, NULL}
};

PostCode PostCodes[] = {
    {0x14FF, "SUCCESS"},
    {0x0075, "SUCCESS"},
    {NULL, NULL}
};

enum MAX6958Registers {
    NoOp = 0x00,
    DecodeMode = 0x01,
    Intensity = 0x02,
    ScanLimit = 0x03,
    Configuration = 0x04,
    FactoryReserved = 0x05,
    GpIo = 0x06,
    DisplayTest = 0x07,
    ReadKeyDebounced = 0x08,
    ReadKeyPressed = 0x0C,
    Digit0 = 0x20,
    Digit1 = 0x21,
    Digit2 = 0x22,
    Digit3 = 0x23,
    Segments = 0x24
};

uint8_t registers[REGISTER_SIZE];
bool newData = false;
uint16_t currentCode = 0;

void receiveEvent(int howMany) {
    int reg = -1;
    while (Wire.available() && reg < REGISTER_SIZE) {
        if (reg == -1) {
            // First byte of a packet is the CMD / target register address
            reg = Wire.read();
        } else {
            registers[reg] = Wire.read();
            if (reg == Segments) {
                // Signal that we have a new POST code
                currentCode = 0;
                currentCode |= registers[Digit0] & 0x0F;
                currentCode |= (registers[Digit1] & 0x0F) << 4;
                currentCode |= (registers[Digit2] & 0x0F) << 8;
                currentCode |= (registers[Digit3] & 0x0F) << 12;
            }
            // After each byte the target register address is automatically incremented
            // This allow communication to be more dense
            // See: MAX6958 Datasheet, page 9 -> Command Address Autoincrementing
            reg++;
        }
    }
    newData = true;
}

const char* getNameForPostcode(uint16_t code) {
    PPostCode pPostCodes = PostCodes;
    while (pPostCodes->name != NULL) {
        if (pPostCodes->code == code) {
            return pPostCodes->name;
        }
        pPostCodes++;
    }

    return NULL;
}

const char* getDeviceNameForAddress(uint8_t addr) {
    PI2cDevice pKnownDevices = KnownDevices;
    while (pKnownDevices->name != NULL) {
        if (pKnownDevices->address == addr) {
            return pKnownDevices->name;
        }
        pKnownDevices++;
    }

    return "<UNKNOWN>";
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

        const char *deviceName = getDeviceNameForAddress(addr);
        if (error == 0)
        {
            Serial.printf("I2C device found at address 0x%02x [%s]\r\n", addr, deviceName);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.printf("Unknown error at address 0x%02x [%s]\r\n", addr, deviceName);
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
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveEvent);
    Serial.printf("Slave Address: 0x%02x\r\n", SLAVE_ADDRESS);
}

void printHelp() {
    Serial.println("\r\nAvailable commands:");
    Serial.println("  post    - Start POST code monitoring");
    Serial.println("  scan    - Scan for I2C devices");
    Serial.println("  help    - Show this help message");
    Serial.println("  CTRL+C  - Exit current mode and return to REPL");
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
                    setupMax6958ASlave();
                    Serial.println("Entering POST monitoring mode. Press CTRL+C to exit.");
                } else if (inputBuffer == "scan") {
                    currentState = STATE_I2C_SCAN;
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

void setup() {
    // Wait for serial to be ready
    while (!Serial) {
        delay(10);
    }
    Serial.begin(115200);
    Wire.setSDA(PIN_SDA);
    Wire.setSCL(PIN_SCL);
    
    Serial.println("POST Reader I2C - Interactive Mode");
    printHelp();
    Serial.print(">> ");  // Initial REPL prompt
}

void printRegisters() {
    Serial.println(">> REGISTERS");
    for (int i = 0; i < REGISTER_SIZE; i++) {
        int val = registers[i];
        Serial.printf("REG 0x%02X : 0x%02X\r\n", i, val);
    }
}

void printCode(uint16_t code) {
    const char *name = getNameForPostcode(code);

    Serial.printf("CODE (SEG: 0x%02x): 0x%04x",
        registers[Segments],
        code
    );

    if (name != NULL) {
        Serial.printf(" [%s]\r\n", name);
    } else {
        Serial.println();
    }
}

void loop() {
    switch (currentState) {
        case STATE_RETURN_TO_REPL:
            // Shutdown I2C Slave, in case we were in POST monitoring before
            Wire.end();

            currentState = STATE_REPL;
            Serial.println("Returning to REPL...");
            printHelp();
            Serial.print(">> ");  // REPL prompt after returning
            break;

        case STATE_REPL:
            handleRepl();
            break;
            
        case STATE_POST_MONITOR:
            if (newData) {
                newData = false;
            }
            if (currentCode != 0) {
                printCode(currentCode);
                currentCode = 0;
            }
            break;
            
        case STATE_I2C_SCAN:
            scanForAvailableDevices();
            // After scan, jump right back into REPL
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