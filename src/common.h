#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "version.h"

#define __DEBUG__

// If you need to use different I/O pins for I2C, change it here
#define PIN_SDA_XBOX 0 // GPIO0
#define PIN_SCL_XBOX 1 // GPIO1
// ---

/* DISPLAY */
#define PIN_SDA_DISP 6 // GPIO6
#define PIN_SCL_DISP 7 // GPIO7
#define DISP_SCREEN_WIDTH 128
#define DISP_SCREEN_HEIGHT 32
#define SSD1306_DISP_ADDRESS 0x3C

/* MAX6958 emulation */
#define MAX6958_ADDRESS 0x38
#define MAX6958_REGISTER_SIZE 0x25

/* POST Code storage */
#define POST_MAX_QUEUE_SIZE 30

#define CTRL_C 3

#if defined(__DEBUG__)
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
    STATE_DISPLAY_ROTATE
};

typedef struct {
    uint16_t code;
    const char *name;
} PostCode, *PPostCode;

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
    {NULL, NULL}
};

static PostCode PostCodes[] = {
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

static const char* getDeviceNameForI2cAddress(uint8_t addr) {
    PI2cDevice pKnownDevices = KnownDevices;
    while (pKnownDevices->name != NULL) {
        if (pKnownDevices->address == addr) {
            return pKnownDevices->name;
        }
        pKnownDevices++;
    }

    return "<UNKNOWN>";
}

static const char* getNameForPostcode(uint16_t code) {
    PPostCode pPostCodes = PostCodes;
    while (pPostCodes->name != NULL) {
        if (pPostCodes->code == code) {
            return pPostCodes->name;
        }
        pPostCodes++;
    }

    return NULL;
}

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