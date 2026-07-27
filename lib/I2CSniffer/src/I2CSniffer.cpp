#include "I2CSniffer.h"

#include <Arduino.h>

// Only ESP32's Arduino core defines IRAM_ATTR (interrupt handlers there must
// live in IRAM); RP2040/Teensy cores don't need or define it.
#if !defined(IRAM_ATTR)
#define IRAM_ATTR
#endif

I2CSniffer *I2CSniffer::_activeInstance = nullptr;

I2CSniffer::I2CSniffer(uint8_t targetAddress, uint8_t sdaPin, uint8_t sclPin) :
    _targetAddress(targetAddress),
    _sdaPin(sdaPin),
    _sclPin(sclPin),
    _queue(sizeof(RawEvent), QUEUE_SIZE, FIFO)
{}

void I2CSniffer::onEvent(EventHandler handler, void *userData) {
    _handler = handler;
    _userData = userData;
}

void I2CSniffer::begin() {
    _activeInstance = this;

    // Claim the pins as plain GPIO inputs explicitly - don't assume
    // whatever the pins were previously used for (e.g. a hardware I2C
    // peripheral released via Wire.end()) has fully reverted their
    // function-select.
    pinMode(_sdaPin, INPUT);
    pinMode(_sclPin, INPUT);

    _currentByte = 0;
    _bitCount = 0;
    _expectingAddress = true;
    _lastWasWrite = true;
    _targetActive = false;
    _droppedEvents = 0;
    _queue.clean();

    attachInterrupt(digitalPinToInterrupt(_sclPin), onSclRiseTrampoline, RISING);
    attachInterrupt(digitalPinToInterrupt(_sdaPin), onSdaChangeTrampoline, CHANGE);
}

void I2CSniffer::end() {
    detachInterrupt(digitalPinToInterrupt(_sclPin));
    detachInterrupt(digitalPinToInterrupt(_sdaPin));
    if (_activeInstance == this) _activeInstance = nullptr;
}

void I2CSniffer::update() {
    RawEvent evt;
    while (_queue.pop(&evt)) {
        if (_handler) _handler((I2CSnifferEvent)evt.type, evt.byte, _userData);
    }
}

void I2CSniffer::pushEvent(I2CSnifferEvent type, uint8_t byte) {
    RawEvent evt{(uint8_t)type, byte};
    if (!_queue.push(&evt)) {
        _droppedEvents++;
    }
}

void IRAM_ATTR I2CSniffer::handleSclRise() {
    bool sda = digitalRead(_sdaPin);

    if (_bitCount < 8) {
        _currentByte = (uint8_t)((_currentByte << 1) | (sda ? 1 : 0));
        _bitCount++;
        return;
    }

    // 9th rising edge = ACK/NACK bit; the byte itself is already complete
    // and its value doesn't affect classification.
    _bitCount = 0;
    if (_expectingAddress) {
        bool isRead = (_currentByte & 0x01) != 0;
        uint8_t addr = (uint8_t)(_currentByte >> 1);
        _targetActive = (addr == _targetAddress);
        _lastWasWrite = !isRead;
        if (_targetActive) {
            pushEvent(isRead ? I2C_SNIFF_ADDRESS_READ : I2C_SNIFF_ADDRESS_WRITE, addr);
        }
        _expectingAddress = false;
    } else if (_targetActive) {
        pushEvent(_lastWasWrite ? I2C_SNIFF_DATA_WRITE : I2C_SNIFF_DATA_READ, _currentByte);
    }
    _currentByte = 0;
}

void IRAM_ATTR I2CSniffer::handleSdaChange() {
    // START/STOP are only meaningful while SCL is high; SDA changing while
    // SCL is low is just normal mid-bit data setup, handled by handleSclRise().
    if (digitalRead(_sclPin) != HIGH) return;

    bool sda = digitalRead(_sdaPin);
    // Transaction boundaries are reported regardless of target match - the
    // next address phase (handleSclRise()) decides whether what follows a
    // START is relevant.
    pushEvent(sda ? I2C_SNIFF_STOP : I2C_SNIFF_START, 0);
    _bitCount = 0;
    _currentByte = 0;
    _expectingAddress = true;
}

void IRAM_ATTR I2CSniffer::onSclRiseTrampoline() {
    if (_activeInstance) _activeInstance->handleSclRise();
}

void IRAM_ATTR I2CSniffer::onSdaChangeTrampoline() {
    if (_activeInstance) _activeInstance->handleSdaChange();
}
