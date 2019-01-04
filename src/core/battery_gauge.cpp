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
constexpr uint8_t STC3100_REGISTERS_START = 0x0;
constexpr uint8_t STC3100_RAM_START = 32;

constexpr float STC3100_VOLTAGE_LSB = 2.44f; // mv
constexpr float STC3100_CURRENT_RSENSE = 30.0f;
constexpr float STC3100_CURRENT_LSB = 11.77f; // uV
constexpr float STC3100_CHARGE_RSENSE = 30.0f;
constexpr float STC3100_CHARGE_LSB = 6.70f; // uV.h

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

    bool read() {
        Wire.beginTransmission(STC3100_ADDRESS);
        Wire.write(STC3100_REGISTERS_START);
        Wire.endTransmission();
        Wire.requestFrom(STC3100_ADDRESS, sizeof(bytes));

        for (size_t i = 0; i < sizeof(bytes); ++i) {
            bytes[i] = Wire.read();
        }

        return true;
    }
} registers_t;

typedef union {
    uint8_t bytes[32];

    struct __attribute__ ((packed)) {
        int32_t counter;
        float ref_voltage;
        float coulombs;
    };

    bool read() {
        Wire.beginTransmission(STC3100_ADDRESS);
        Wire.write(STC3100_RAM_START);
        Wire.endTransmission();
        Wire.requestFrom(STC3100_ADDRESS, sizeof(bytes));

        for (size_t i = 0; i < sizeof(bytes); ++i) {
            bytes[i] = Wire.read();
        }

        return true;
    }

    bool write() {
        Wire.beginTransmission(STC3100_ADDRESS);
        Wire.write(STC3100_RAM_START);

        for (size_t i = 0; i < sizeof(bytes); ++i) {
            Wire.write(bytes[i]);
        }

        Wire.endTransmission();

        return true;
    }
} ram_t;

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

BatteryGauge::BatteryReading BatteryGauge::read() {
    registers_t registers;
    ram_t ram;

    if (!registers.read()) {
        return { };
    }

    if (!ram.read()) {
        return { };
    }

    // Truncate and convert to signed.
    registers.current &= 0x3fff;
    if (registers.current >= 0x2000) registers.current -= 0x4000;

    auto ma = (float)(registers.current) * STC3100_CURRENT_LSB / STC3100_CURRENT_RSENSE;

    // Truncate and convert to signed.
    registers.voltage &= 0x0fff;
    if (registers.voltage >= 0x0800) registers.voltage -= 0x1000;

    auto voltage = (float)(registers.voltage) * STC3100_VOLTAGE_LSB;

    // Truncate and convert to signed.
    registers.charge &= 0x0fff;
    if (registers.charge >= 0x0800) registers.charge -= 0x1000;

    auto coulombs = (float)(registers.charge) * STC3100_CHARGE_LSB / STC3100_CHARGE_RSENSE;

    auto counter = registers.counter_u16;

    if (ram.counter == 0) {
        ram.ref_voltage = voltage;
    }
    ram.coulombs = coulombs;
    ram.counter++;

    if (!ram.write()) {
        return { };
    }


    // TODO: If the coulomb counter has levelled off then the battery is effectively charged.

    return {
        ma > 0.0f,
        voltage,
        ma,
        coulombs,
        counter,
    };
}

}
