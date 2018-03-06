#include <Wire.h>
#include "wiring_private.h"

#include "two_wire.h"

namespace fk {

bool TwoWireBus::begin(uint32_t speed) {
    bus->begin();

    if (speed > 0) {
        Wire.setClock(speed);
    }

    #ifndef FK_NATURALIST
    if (bus == &Wire11and13) {
        pinPeripheral(11, PIO_SERCOM);
        pinPeripheral(13, PIO_SERCOM);
    }
    #endif
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

    #ifndef FK_NATURALIST
    if (bus == &Wire11and13) {
        pinPeripheral(11, PIO_SERCOM);
        pinPeripheral(13, PIO_SERCOM);
    }
    #endif
    if (bus == &Wire4and3) {
        pinPeripheral(4, PIO_SERCOM_ALT);
        pinPeripheral(3, PIO_SERCOM_ALT);
    }

    return true;
}

void TwoWireBus::end() {
    bus->end();
}

bool TwoWireBus::send(uint8_t address, const char *ptr) {
    if (address > 0) {
        bus->beginTransmission(address);
        bus->write(ptr);
        switch (bus->endTransmission()) {
        case 0:
            return true;
        default:
            return false;
        }
    } else {
        bus->write(ptr);
    }

    return true;
}

bool TwoWireBus::send(uint8_t address, const void *ptr, size_t size) {
    if (address > 0) {
        bus->beginTransmission(address);
        bus->write((uint8_t *)ptr, size);
        switch (bus->endTransmission()) {
        case 0:
            return true;
        default:
            return false;
        }
    } else {
        bus->write((uint8_t *)ptr, size);
    }

    return true;
}

size_t TwoWireBus::receive(uint8_t address, uint8_t *ptr, size_t size) {
    bus->requestFrom(address, FK_MODULE_PROTOCOL_MAX_MESSAGE);

    size_t bytes = 0;
    while (bus->available()) {
        ptr[bytes++] = bus->read();
        if (bytes == size) {
            break;
        }
    }

    return bytes;
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
        debugfpln("I2C", "Flushed %d bytes", flushed);
    }
}

Peripherals peripherals;

#ifndef FK_NATURALIST
TwoWire Wire11and13{ &sercom1, 11, 13 };
#endif

TwoWire Wire4and3{ &sercom2, 4, 3 };

extern "C" {

#ifndef FK_NATURALIST
void SERCOM1_Handler(void) {
    fk::Wire11and13.onService();
}
#endif

void SERCOM2_Handler(void) {
    fk::Wire4and3.onService();
}

}

}
