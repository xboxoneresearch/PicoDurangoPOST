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

#else
#error "Unsupported platform - only RP2040, ESP32 and Teensy 4.x are supported"
#endif
