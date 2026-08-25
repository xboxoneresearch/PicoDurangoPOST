#pragma once

// Passive, bit-banged I2C sniffer for a single target address - watches
// SDA/SCL as plain GPIO inputs (no hardware I2C peripheral, no ACK, no bus
// participation) and reconstructs the bus traffic addressed to
// `targetAddress`. Useful when this MCU needs to observe a transaction
// between some other bus master and some other target device it doesn't
// own and can't safely impersonate (e.g. a chip already living at that
// address on a shared bus).
//
// API is deliberately similar to TwoWire's onReceive()/onRequest()
// callback-registration style. Unlike TwoWire though, the registered
// handler fires from update() (call it regularly from your main loop),
// not from interrupt context - bytes are assembled in GPIO ISRs, but
// running arbitrary callback code (printing, pushing to other queues,
// etc.) from interrupt context isn't safe or portable, so that work is
// deferred to mainline code.
//
// Only one instance can be begin()-active at a time: attachInterrupt()
// needs a plain function pointer, so this class routes through a single
// static "active instance" pointer under the hood. Fine for watching one
// bus/address; concurrently active sniffers aren't supported.

#include <cstdint>
#include <cppQueue.h>

enum I2CSnifferEvent : uint8_t {
    I2C_SNIFF_START,
    I2C_SNIFF_ADDRESS_WRITE,
    I2C_SNIFF_ADDRESS_READ,
    I2C_SNIFF_DATA_WRITE,
    I2C_SNIFF_DATA_READ,
    I2C_SNIFF_STOP,
};

class I2CSniffer {
public:
    typedef void (*EventHandler)(I2CSnifferEvent event, uint8_t byte, void *userData);

    I2CSniffer(uint8_t targetAddress, uint8_t sdaPin, uint8_t sclPin);

    // Registers the callback invoked (from update()) for bus events.
    // START/STOP are always reported (they're transaction boundaries, not
    // address-specific); ADDRESS/DATA events are only reported for
    // transactions whose address phase matched targetAddress.
    void onEvent(EventHandler handler, void *userData = nullptr);

    void begin();
    void end();

    // Dispatches any bus events accumulated since the last call to the
    // registered handler. Call regularly (e.g. every main-loop iteration).
    void update();

    // Count of raw bus events dropped because the internal ISR->update()
    // queue was full - non-zero means real bus traffic is outrunning
    // update()'s polling rate.
    uint32_t droppedEvents() const { return _droppedEvents; }

    uint8_t targetAddress() const { return _targetAddress; }

private:
    struct RawEvent {
        uint8_t type;
        uint8_t byte;
    };
    static constexpr uint16_t QUEUE_SIZE = 256;

    uint8_t _targetAddress;
    uint8_t _sdaPin;
    uint8_t _sclPin;
    EventHandler _handler = nullptr;
    void *_userData = nullptr;
    cppQueue _queue;

    // Bit-assembly state, touched from ISR context - non-nested,
    // single-priority GPIO interrupts (the Arduino default on every
    // supported core) make plain `volatile` sufficient here.
    volatile uint8_t _currentByte = 0;
    volatile uint8_t _bitCount = 0;
    volatile bool _expectingAddress = true;
    volatile bool _lastWasWrite = true;
    volatile bool _targetActive = false;
    volatile uint32_t _droppedEvents = 0;

    void pushEvent(I2CSnifferEvent type, uint8_t byte);
    void handleSclRise();
    void handleSdaChange();

    static void onSclRiseTrampoline();
    static void onSdaChangeTrampoline();
    static I2CSniffer *_activeInstance;
};
