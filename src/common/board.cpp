#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "debug.h"
#include "board.h"
#include "two_wire.h"

namespace fk {

void Board::disable_cs(uint8_t pin) {
    pinMode(pin, INPUT);
}

void Board::enable_cs(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void Board::low(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Board::high(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

SpiWrapper Board::spi() {
    return SpiWrapper::spi();
}

TwoWireWrapper Board::i2c1() {
    return TwoWireWrapper::i2c1();
}

TwoWireWrapper Board::i2c2() {
    return TwoWireWrapper::i2c2();
}

SpiWrapper SpiWrapper::spi() {
    return SpiWrapper { &SPI };
}

void SpiWrapper::begin() {
    reinterpret_cast<SPIClass*>(ptr_)->begin();
}

void SpiWrapper::end() {
    reinterpret_cast<SPIClass*>(ptr_)->end();

    pinMode(PIN_SPI_MISO, INPUT);
    pinMode(PIN_SPI_MOSI, INPUT);
    pinMode(PIN_SPI_SCK, INPUT);
}

TwoWireWrapper TwoWireWrapper::i2c1() {
    return { &Wire };
}

TwoWireWrapper TwoWireWrapper::i2c2() {
    return { &Wire4and3 };
}

void TwoWireWrapper::begin() {
    TwoWireBus bus{ *reinterpret_cast<TwoWire*>(ptr_) };
    bus.begin();
}

void TwoWireWrapper::end() {
    TwoWireBus bus{ *reinterpret_cast<TwoWire*>(ptr_) };
    bus.end();
}

}
