#pragma once

#include <Arduino.h>

#define SEGMENT_FLAVOR_MASK 0xF0
#define SEGMENT_INDEX_MASK  0x0F

enum CodeIndex: uint8_t {
    CODE_IDX_CPU = 0,
    CODE_IDX_SP,
    CODE_IDX_SMC,
    CODE_IDX_OS,
    CODE_IDX_MAX,
    CODE_IDX_INVALID,
};

// Indicated by Segments-register
enum CodeFlavor: uint8_t {
    CODE_FLAVOR_CPU = 0x10,
    CODE_FLAVOR_SP =  0x30,
    CODE_FLAVOR_SMC = 0x70,
    CODE_FLAVOR_OS =  0xF0,
};

typedef struct {
    uint64_t code;
    CodeFlavor flavor;
    uint64_t timestamp;
} SegmentData, *PSegmentData;

class SegmentByte {
public:
    SegmentByte(uint8_t segByte) {
        byte = segByte;
    }

    CodeFlavor flavor() {
        return (CodeFlavor)(byte & SEGMENT_FLAVOR_MASK);
    }

    uint8_t index() {
        return (byte & SEGMENT_INDEX_MASK);
    }
private:
    uint8_t byte;
};

static inline CodeIndex getCodeIndexForFlavor(CodeFlavor flavor) {
    switch (flavor) {
        case CODE_FLAVOR_CPU:
            return CODE_IDX_CPU;
        case CODE_FLAVOR_SP:
            return CODE_IDX_SP;
        case CODE_FLAVOR_SMC:
            return CODE_IDX_SMC;
        case CODE_FLAVOR_OS:
            return CODE_IDX_OS;
        default:
            return CODE_IDX_INVALID;
    }
}

static inline const char* getStringForCodeFlavor(CodeFlavor flavor) {
    switch (flavor) {
        case CODE_FLAVOR_CPU:
            return "CPU";
        case CODE_FLAVOR_SP:
            return "SP ";
        case CODE_FLAVOR_SMC:
            return "SMC";
        case CODE_FLAVOR_OS:
            return "OS ";
        default:
            return "??";
    }
}

#define POST_CODE_WORD_COUNT 4

// Merges up to 4x 16-bit digit-group values into one 64-bit code.
static inline uint64_t assembleCode(const uint16_t words[POST_CODE_WORD_COUNT]) {
    uint64_t code = 0;
    for (int idx = 0; idx < POST_CODE_WORD_COUNT; idx++) {
        code |= (uint64_t)words[idx] << (16 * idx);
    }
    return code;
}
