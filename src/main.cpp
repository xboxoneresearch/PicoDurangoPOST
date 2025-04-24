#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x38
#define REGISTER_SIZE 0x25

typedef struct {
    uint8_t address;
    const char *name;
} I2cDevice, *PI2cDevice;

I2cDevice KnownDevices[] = {
    // OG Xbox One (PHAT) 
    {0x15, "V_NBCORE (U5F3)"},
    {0x16, "V5_P0 (U3E2)"},
    {0x18, "V_MEMIOAB (U9F3 or U9F2)"},
    {0x19, "V_MEMIOCD (U6C1)"},
    {0x20, "V_GFXCORE / V_CPUCORE (U9C2)"},
    {0x38, "MAX6958/9A"},
    {0x39, "MAX6958/9B"},
    {0x5A, "RF Unit"},
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
bool newCode = false;

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
                newCode = true;
            }
            // After each byte the target register address is automatically incremented
            // This allow communication to be more dense
            // See: MAX6958 Datasheet, page 9 -> Command Address Autoincrementing
            reg++;
        }
    }
    newData = true;
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
            Serial.printf("I2C device found at address 0x%02x [%s]\n", addr, deviceName);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.printf("Unknown error at address 0x%02x [%s]\n", addr, deviceName);
        }
    }
    if (nDevices > 0)
        Serial.printf("Found %d devices on I2C Bus\n", nDevices);
    else
        Serial.println("No devices found on I2C bus...");

    Wire.end();
}

void setupMax6958ASlave() {
    // As I2C Slave, simulate MAX6958
    Serial.println("POST Monitor (I2C Slave as MAX6958A) started");
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveEvent);
    Serial.printf("Slave Address: 0x%02x\n", SLAVE_ADDRESS);
}

void setup() {
    while (!Serial) {
        delay(10);
    }
    Serial.begin(115200);

    scanForAvailableDevices();

    Serial.println("");
    setupMax6958ASlave();
}

void printRegisters() {
    Serial.println(">> REGISTERS");
    for (int i = 0; i < REGISTER_SIZE; i++) {
        int val = registers[i];
        Serial.printf("REG 0x%02X : 0x%02X\n", i, val);
    }
}

void printCode() {
    Serial.printf("CODE (SEG: 0x%02x): 0x%x%x%x%x\n",
        registers[Segments],
        registers[Digit3],
        registers[Digit2],
        registers[Digit1],
        registers[Digit0]
    );
}

void loop() {
    if (newData) {
        // printRegisters();
        newData = false;
    }
    if (newCode) {
        printCode();
        newCode = false;
    }
} 