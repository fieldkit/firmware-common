#include <Arduino.h>
#include <Wire.h>

#include "wiring_private.h"
#include "two_wire.h"
#include "tuning.h"
#include "performance.h"

namespace fk {

static inline bool check_end_transaction(uint32_t value) {
    return value == 0;
}

bool TwoWireBus::begin(uint32_t speed) {
    bus->begin();

    if (speed > 0) {
        bus->setClock(speed);
    }

    if (bus == &Wire4and3) {
        pinPeripheral(4, PIO_SERCOM_ALT);
        pinPeripheral(3, PIO_SERCOM_ALT);
    }

    return true;
}

bool TwoWireBus::begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest) {
    bus->begin(address);
    bus->onReceive(onReceive);
    bus->onRequest(onRequest);

    if (bus == &Wire4and3) {
        pinPeripheral(4, PIO_SERCOM_ALT);
        pinPeripheral(3, PIO_SERCOM_ALT);
    }

    return true;
}

void TwoWireBus::end() {
    bus->end();

    if (bus == &Wire) {
        pinMode(I2C_PIN_SDA1, INPUT);
        pinMode(I2C_PIN_SDA1, INPUT);
    }
    else if (bus == &Wire4and3) {
        pinMode(I2C_PIN_SDA2, INPUT);
        pinMode(I2C_PIN_SDA2, INPUT);
    }
}

bool TwoWireBus::send(uint8_t address, const char *ptr) {
    if (address > 0) {
        bus->beginTransmission(address);
        bus->write(ptr);
        return check_end_transaction(bus->endTransmission());
    } else {
        bus->write(ptr);
    }

    return true;
}

bool TwoWireBus::send(uint8_t address, uint8_t value) {
    bus->beginTransmission(address);
    bus->write(value);
    return check_end_transaction(bus->endTransmission());
}

bool TwoWireBus::write(uint8_t address, uint8_t reg, uint8_t value) {
    bus->beginTransmission(address);
    bus->write(reg);
    bus->write(value);
    return check_end_transaction(bus->endTransmission());
}

bool TwoWireBus::write(uint8_t address, uint8_t reg, uint16_t value) {
    bus->beginTransmission(address);
    bus->write(reg);
    bus->write((value >> 8) & 0xff);
    bus->write((value) & 0xff);
    return check_end_transaction(bus->endTransmission());
}

uint8_t TwoWireBus::read(uint8_t address, uint8_t reg) {
    bus->beginTransmission(address);
    bus->write(reg);
    if (!check_end_transaction(bus->endTransmission())) {
            return 0xff;
    }

    bus->requestFrom(address, 1);
    auto value = bus->read();
    if (!check_end_transaction(bus->endTransmission())) {
        return 0xff;
    }
    return value;
}

bool TwoWireBus::send(uint8_t address, const void *ptr, size_t size) {
    if (address > 0) {
        bus->beginTransmission(address);
        bus->write((uint8_t *)ptr, size);
        return check_end_transaction(bus->endTransmission());
    } else {
        bus->write((uint8_t *)ptr, size);
    }

    return true;
}

size_t TwoWireBus::receive(uint8_t address, uint8_t *ptr, size_t size) {
    bus->requestFrom(address, size);

    size_t bytes = 0;
    while (bus->available()) {
        ptr[bytes++] = bus->read();
        if (bytes == size) {
            break;
        }
    }

    return bytes;
}

size_t TwoWireBus::receive(uint8_t address, TwoWire16 &data) {
    bus->requestFrom(address, sizeof(data));
    for (auto i = sizeof(data); i > 0; i--) {
        data.bytes[i - 1] = bus->read();
    }

    return check_end_transaction(bus->endTransmission());
}

size_t TwoWireBus::receive(uint8_t address, TwoWire32 &data) {
    bus->requestFrom(address, sizeof(data));
    for (auto i = sizeof(data); i > 0; i--) {
        data.bytes[i - 1] = bus->read();
    }

    return check_end_transaction(bus->endTransmission());
}

uint8_t TwoWireBus::requestFrom(uint8_t address, size_t quantity, bool stopBit) {
    return bus->requestFrom(address, quantity, stopBit);
}

uint32_t TwoWireBus::available() {
    return bus->available();
}

uint8_t TwoWireBus::endTransmission() {
    return bus->endTransmission();
}

uint8_t TwoWireBus::read() {
    return bus->read();
}

size_t TwoWireBus::read(uint8_t *ptr, size_t size, size_t bytes) {
    fk_assert(bytes < size);
    for (size_t i = 0; i < bytes; ++i) {
        ptr[i] = bus->read();
    }
    return bytes;
}

void TwoWireBus::flush() {
    if (bus->available()) {
        auto flushed = 0;
        while (bus->available()) {
            flushed++;
            bus->read();
        }
        loginfof("I2C", "Flushed %d bytes", flushed);
    }
}

Peripherals peripherals;

TwoWire Wire4and3{ &sercom2, 4, 3 };

extern "C" {

void SERCOM2_Handler(void) {
    fk::Wire4and3.onService();
}

}

}
