#include <string.h>

#include "socpost_decode.h"
#include "I2CSniffer.h"

void SocPostDecoder::reset() {
    regAddr = 0;
}

void SocPostDecoder::handleRead(uint8_t byte, SocPostCode &out) {
    uint8_t reg = regAddr;
    regAddr++;

    if (reg >= SOC_POST_REG0 && reg <= SOC_POST_REG3) {
        // substract base register offset, to yield target position in buffer
        // f.e. 0xC3 -> 0x03
        codeCache[reg - SOC_POST_REG0] = byte;

        if (reg == SOC_POST_REG0 && !(byte & 0x80)) {
            out.postCode = byte;
            out.isExtendedCode = false;
        } else if (reg == SOC_POST_REG3) {
            memcpy(&out.postCode, codeCache, sizeof(out.postCode));
            out.isExtendedCode = true;
        }
    }
}

void SocPostDecoder::handleEvent(I2CSnifferEvent type, uint8_t byte, SocPostCode &out) {
    switch (type) {
        case I2C_SNIFF_START:
        case I2C_SNIFF_STOP:
        case I2C_SNIFF_ADDRESS_WRITE:
        case I2C_SNIFF_ADDRESS_READ:
            break;
        case I2C_SNIFF_DATA_WRITE:
            regAddr = byte;
            break;
        case I2C_SNIFF_DATA_READ:
            handleRead(byte, out);
            break;
    }
}