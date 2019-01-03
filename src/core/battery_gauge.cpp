#include <Wire.h>
#include <Arduino.h>

#include "battery_gauge.h"

namespace fk {

// https://www.st.com/content/ccc/resource/technical/document/application_note/fa/c2/de/32/e8/c9/4e/28/CD00248578.pdf/files/CD00248578.pdf/jcr:content/translations/en.CD00248578.pdf

constexpr uint8_t STC3100_ADDRESS = 0x70;
constexpr uint8_t STC3100_REGISTER_ID = 24;
constexpr uint8_t STC3100_REGISTER_MODE = 0;
constexpr uint8_t STC3100_REGISTER_MODE_VALUE_ENABLED = 0x10;
constexpr uint8_t STC3100_REGISTER_MODE_VALUE_DISABLED = 0;
constexpr uint8_t STC3100_REGISTER_CONTROL = 1;
constexpr uint8_t STC3100_REGISTER_CONTROL_VALUE_RESET = 0x2;
constexpr uint8_t STC3100_REGISTER_CHARGE = 2;
constexpr uint8_t STC3100_REGISTER_CURRENT = 6;
constexpr uint8_t STC3100_REGISTER_VOLTAGE = 8;
constexpr uint8_t STC3100_DEVICE_PART = 0x10;

constexpr float STC3100_VOLTAGE_LSB = 2.44f; // mv
constexpr float STC3100_CURRENT_RSENSE = 30.0f;
constexpr float STC3100_CURRENT_LSB = 11.77f; // uV
constexpr float STC3100_CHARGE_RSENSE = 30.0f;
constexpr float STC3100_CHARGE_LSB = 6.70f; // uV.h

bool BatteryGauge::available() {
    uint8_t id[8] = { 0 };

    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(STC3100_REGISTER_ID);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, (uint8_t)sizeof(id));

    for (size_t i = 0; i < sizeof(id); ++i) {
        id[i] = Wire.read();
    }

    if (id[0] != STC3100_DEVICE_PART) {
        return false;
    }

    return enable();
}

bool BatteryGauge::enable() {
    // Read this register to clear GG_EOC and VTM_EOC
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(STC3100_REGISTER_CONTROL);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, 1);
    Wire.read(); // Ignore

    // Clear accumulator, counter and PORDET.
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(STC3100_REGISTER_CONTROL);
    Wire.write(STC3100_REGISTER_CONTROL_VALUE_RESET);
    Wire.endTransmission();

    // Just write the whole register, for now. The other values in there are 0.
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(STC3100_REGISTER_MODE);
    Wire.write(STC3100_REGISTER_MODE_VALUE_ENABLED);
    Wire.endTransmission();

    return true;
}

bool BatteryGauge::disable() {
    // Just write the whole register, for now. The other values in there are 0.
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(STC3100_REGISTER_MODE);
    Wire.write(STC3100_REGISTER_MODE_VALUE_DISABLED);
    Wire.endTransmission();

    return true;
}

typedef union {
    uint8_t bytes[32];

    struct __attribute__ ((packed)) {
        uint8_t mode;
        uint8_t control_status;
        int16_t charge;
        uint16_t counter_u16;
        int16_t current;
        int16_t voltage;
        uint8_t temperature_low;
        uint8_t temperature_high;
        uint8_t reserved[13];
        uint8_t id0;
        uint8_t id1;
        uint8_t id2;
        uint8_t id3;
        uint8_t id4;
        uint8_t id5;
        uint8_t id6;
        uint8_t id7;
    };
} registers_t;

BatteryGauge::BatteryReading BatteryGauge::read() {
    registers_t registers;

    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, sizeof(registers));

    for (size_t i = 0; i < sizeof(registers_t); ++i) {
        registers.bytes[i] = Wire.read();
    }

    // Truncate and convert to signed.
    registers.current &= 0x3fff;
    if (registers.current >= 0x2000) registers.current -= 0x4000;

    auto current = (float)(registers.current) * STC3100_CURRENT_LSB / STC3100_CURRENT_RSENSE;

    // Truncate and convert to signed.
    registers.voltage &= 0x0fff;
    if (registers.voltage >= 0x0800) registers.voltage -= 0x1000;

    auto voltage = (float)(registers.voltage) * STC3100_VOLTAGE_LSB;

    // Truncate and convert to signed.
    registers.charge &= 0x0fff;
    if (registers.charge >= 0x0800) registers.charge -= 0x1000;

    auto charge = (float)(registers.charge) * STC3100_CHARGE_LSB / STC3100_CHARGE_RSENSE;

    auto counter = registers.counter_u16;

    return {
        voltage,
        current,
        charge,
        counter,
    };
}

}
