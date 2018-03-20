#pragma once

#include <cstring>
#include <cstdint>

class TwoWire {
public:
    void begin() {
    }

    void beginTransmission(uint8_t) {
    }
    uint8_t endTransmission(bool stopBit) {
        return 0;
    }
    uint8_t endTransmission(void) {
        return 0;
    }

    uint8_t requestFrom(uint8_t address, size_t quantity, bool stopBit) {
        return 0;
    }
    uint8_t requestFrom(uint8_t address, size_t quantity) {
        return 0;
    }

    size_t send(uint8_t data) {
        return 0;
    }
    size_t write(uint8_t data) {
        return 0;
    }
    size_t write(const uint8_t * data, size_t quantity) {
        return 0;
    }

    virtual int available(void) {
        return 0;
    }
    virtual int receive(void) {
        return 0;
    }
    virtual int read(void) {
        return 0;
    }
    virtual int peek(void) {
        return 0;
    }
    virtual void flush(void) {
    }
    void onReceive(void(*)(int)) {
    }
    void onRequest(void(*)(void)) {
    }
};

extern TwoWire Wire;

