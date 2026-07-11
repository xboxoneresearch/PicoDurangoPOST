#pragma once

// RP2040 (arduino-pico) vs ESP32 (arduino-esp32) differences, isolated here:
// - 64-bit microsecond clock
// - reboot into flashing mode
// - core1: arduino-pico calls setup1()/loop1() natively, ESP32 has no such
//   hook so we run the same functions from a FreeRTOS task pinned to core 1.

#include <Arduino.h>

#if defined(ARDUINO_ARCH_RP2040)

static inline uint64_t now_us64() { return time_us_64(); }
static inline void rebootToBootloader() { rp2040.rebootToBootloader(); }
static inline void platformStartCore1() {} // arduino-pico already runs setup1()/loop1()

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

#else
#error "Unsupported platform - only RP2040 and ESP32 are supported"
#endif
