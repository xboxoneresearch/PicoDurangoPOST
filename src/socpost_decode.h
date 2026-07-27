#pragma once

#include <stdint.h>
#include "I2CSniffer.h"

#define SOC_POST_CODE_REG_COUNT 4

constexpr uint8_t SOC_POST_TARGET_ADDR = 0x4C;

enum SocPostRegister : uint8_t {
    SOC_POST_REG0 = 0xC4,
    SOC_POST_REG1,
    SOC_POST_REG2,
    SOC_POST_REG3,
};

struct SocPostCode {
    uint32_t postCode;
    bool isExtendedCode;
};

class SocPostDecoder {
public:
    void reset();
    void handleEvent(I2CSnifferEvent type, uint8_t byte, SocPostCode &out);

private:
    uint8_t regAddr = 0;
    uint8_t codeCache[SOC_POST_CODE_REG_COUNT] = {0};

    void handleRead(uint8_t byte, SocPostCode &out);
};
