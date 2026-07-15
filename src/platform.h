#pragma once

// Platform differences are isolated here:
// - 64-bit microsecond clock
// - reboot into flashing mode
// - core1: arduino-pico calls setup1()/loop1() natively. Platforms without a
//   second physical core (or without one exposed the same way) instead run
//   loop1() from platformPumpCore1(), called once per core0 loop() iteration.

#include <Arduino.h>

#if defined(ARDUINO_ARCH_RP2040)

static inline uint64_t now_us64() { return time_us_64(); }
static inline void rebootToBootloader() { rp2040.rebootToBootloader(); }
static inline void platformStartCore1() {} // arduino-pico already runs setup1()/loop1()
static inline void platformPumpCore1() {}

static inline bool platformSupportsI2C0PinChange() { return true; }

// RP2040/RP2350 GPIO function-select: I2C SDA/SCL alternate with GPIO parity,
// and I2C0 vs I2C1 alternates every other pair ((gpio/2) % 2). Getting this
// wrong doesn't fail gracefully - Wire.setSDA()/setSCL() panic-reboot the
// board on an unsupported pin, so bad input must be rejected before that.
// GPIO0/1 (this board's own default Xbox-bus pins) are valid I2C0 SDA/SCL.
static inline constexpr bool isValidI2C0Pins(uint8_t sda, uint8_t scl) {
    return sda <= 28 && scl <= 28 && (sda % 4) == 0 && (scl % 4) == 1 && scl - sda == 1;
}
static_assert(isValidI2C0Pins(0, 1), "GPIO0/1 must be valid I2C0 pins on RP2040");
static_assert(isValidI2C0Pins(4, 5), "GPIO4/5 must be valid I2C0 pins on RP2040");
static_assert(!isValidI2C0Pins(2, 3), "GPIO2/3 belong to I2C1, not I2C0");
static_assert(!isValidI2C0Pins(1, 0), "SDA/SCL roles are fixed per pin, can't be swapped");

#elif defined(ARDUINO_ARCH_ESP32)

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static inline uint64_t now_us64() { return (uint64_t)esp_timer_get_time(); }

// ponytail: classic ESP32 has no UF2/BOOTSEL flow reachable from firmware,
// flashing goes through esptool's DTR/RTS auto-reset instead. Plain restart
// is the closest equivalent.
static inline void rebootToBootloader() { ESP.restart(); }

void setup1();
void loop1();

static void core1_task(void *) {
    setup1();
    for (;;) {
        loop1();
        vTaskDelay(1); // yield so the core1 idle task can feed the watchdog
    }
}

static inline void platformStartCore1() {
    xTaskCreatePinnedToCore(core1_task, "core1", 4096, NULL, 1, NULL, 1);
}
static inline void platformPumpCore1() {}

static inline bool platformSupportsI2C0PinChange() { return true; }

// ESP32's I2C is routed through the GPIO matrix, so almost any GPIO works
// for either role - the one hard rule is SDA and SCL can't be the same pin.
#if defined(CONFIG_IDF_TARGET_ESP32S3)
static inline constexpr bool isValidI2C0Pin(uint8_t pin) { return pin <= 48; }
#else
// GPIO34-39 are input-only on classic ESP32 and can't drive an open-drain I2C line.
static inline constexpr bool isValidI2C0Pin(uint8_t pin) { return pin <= 39 && !(pin >= 34 && pin <= 39); }
#endif
static inline constexpr bool isValidI2C0Pins(uint8_t sda, uint8_t scl) {
    return sda != scl && isValidI2C0Pin(sda) && isValidI2C0Pin(scl);
}

#elif defined(TEENSYDUINO)

// Teensy 4.x (imxrt1062) is single-core with no bundled RTOS: there's no
// second execution context to run setup1()/loop1() on. The I2C slave
// reception itself (core1_receiveI2cData) is still interrupt-driven and
// runs regardless, so loop1() - which only checks for a cross-thread
// message - is safe to just call inline from core0's loop().
static inline uint64_t now_us64() {
    // micros() is 32-bit and wraps after ~71 minutes; stretch it to 64 bits
    // by tracking wraparounds. Relies on now_us64() being called more often
    // than that, which the main loop() easily does.
    static uint32_t high = 0;
    static uint32_t lastLow = 0;
    uint32_t low = micros();
    if (low < lastLow) high++;
    lastLow = low;
    return ((uint64_t)high << 32) | low;
}

// Jumps to the HalfKay bootloader (same mechanism the Teensy Loader uses).
extern "C" void _reboot_Teensyduino_(void);
static inline void rebootToBootloader() { _reboot_Teensyduino_(); }

void setup1();
void loop1();

static inline void platformStartCore1() { setup1(); }
static inline void platformPumpCore1() { loop1(); }

// Wire/Wire1/Wire2 pins are wired to fixed silicon pads on Teensy 4.x, not
// software-remappable, so there's nothing to validate or change.
static inline bool platformSupportsI2C0PinChange() { return false; }
static inline constexpr bool isValidI2C0Pins(uint8_t, uint8_t) { return false; }

#else
#error "Unsupported platform - only RP2040, ESP32 and Teensy 4.x are supported"
#endif
