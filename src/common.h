#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <cppQueue.h>
#include "display.h"

/* DISPLAY */
#define DISP_SCREEN_WIDTH 128
#define DISP_SCREEN_HEIGHT 32
#define SSD1306_DISP_ADDRESS 0x3C

/* MAX6958 emulation */
#define MAX6958_ADDRESS 0x38
#define MAX6958_REGISTER_SIZE 0x25

/* POST Code storage */
#define POST_MAX_QUEUE_SIZE 30

#define CTRL_C 3

#if DEBUG
#define DBG(format, ...) Serial.printf("[DBG] "#format"\r\n", ##__VA_ARGS__)
#else
#define DBG(x, ...)
#endif

// State machine states
enum State {
    STATE_RETURN_TO_REPL,
    STATE_REPL,
    STATE_POST_MONITOR,
    STATE_I2C_SCAN,
    STATE_TOGGLE_TIMESTAMP,
    STATE_TOGGLE_COLORS,
    STATE_DISPLAY_MIRROR,
    STATE_DISPLAY_ROTATE,
    STATE_CONFIG_SHOW,
    STATE_CONFIG_SAVE,
};

// For communication between core0/1
enum CrossThreadMsg: uint32_t {
    INVALID = 0,
    SCAN_I2C = 1,
    RESET_TIMESTAMP = 2
};

typedef struct {
    uint8_t digits[4];
    uint8_t segments;
    uint64_t timestamp;
} SegmentData, *PSegmentData;

typedef struct {
    uint8_t address;
    const char *name;
} I2cDevice, *PI2cDevice;

static I2cDevice KnownDevices[] = {
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
};

const uint16_t KNOWN_DEVICES_COUNT = sizeof(KnownDevices) / sizeof(I2cDevice);

static const char* getDeviceNameForI2cAddress(uint8_t addr) {
    int pos = 0;
    while (pos < KNOWN_DEVICES_COUNT) {
        if (KnownDevices[pos].address == addr) {
            return KnownDevices[pos].name;
        }
        pos++;
    }

    return "<UNKNOWN>";
}

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
    Segments = 0x24,
};

static const char *getNameForMAX6958Register(uint8_t reg) {
    switch(reg) {
        case NoOp:
            return "NoOp";
        case DecodeMode:
            return "DecodeMode";
        case Intensity:
            return "Intensity";
        case ScanLimit:
            return "ScanLimit";
        case Configuration:
            return "Configuration";
        case FactoryReserved:
            return "FactoryReserved";
        case GpIo:
            return "GpIo";
        case DisplayTest:
            return "DisplayTest";
        case ReadKeyDebounced:
            return "ReadKeyDebounced";
        case ReadKeyPressed:
            return "ReadKeyPressed";
        case Digit0:
            return "Digit0";
        case Digit1:
            return "Digit1";
        case Digit2:
            return "Digit2";
        case Digit3:
            return "Digit3";
        case Segments:
            return "Segments";
        default:
            return "<UNKNOWN>";
    }
}

class RuntimeState {
public:
    RuntimeState();
    ~RuntimeState();

    bool begin();

    inline void setCurrentState(State state) { currentState = state; }
    inline State getCurrentState() { return currentState; }

    inline void setRegister(uint8_t reg, uint8_t value) { registers[reg] = value; }
    inline uint8_t getRegister(uint8_t reg) { return registers[reg]; }

    inline void resetTimestamp() { timestamp = time_us_64(); }
    inline uint64_t getTimestamp() { return timestamp; }

    inline Display *display() { return &_display; }

    inline bool isPostCodeQueueFull() { return _postCodeQueue.isFull(); }
    inline bool isPostCodeQueueEmpty() { return _postCodeQueue.isEmpty(); }
    inline void pushPostCode(SegmentData* segData) { _postCodeQueue.push(segData); }
    inline bool popPostCode(SegmentData *segDataOut) {
        if (_postCodeQueue.getCount() > 0) {
            return _postCodeQueue.pop((SegmentData*)segDataOut);
        }
        return false;
    }
    inline void clearPostCodeQueue() { _postCodeQueue.clean(); }
private:
    uint64_t timestamp = time_us_64();
    State currentState = STATE_POST_MONITOR;
    uint8_t registers[MAX6958_REGISTER_SIZE];
    bool initialized = false;

    Display  _display;
    // Create a queue for POST codes
    cppQueue _postCodeQueue;
};