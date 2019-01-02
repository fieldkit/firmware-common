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

    struct {
        uint8_t mode;
        uint8_t control_status;
        uint8_t charge_low;
        uint8_t charge_high;
        uint8_t counter_low;
        uint8_t counter_high;
        uint8_t current_low;
        uint8_t current_high;
        uint8_t voltage_low;
        uint8_t voltage_high;
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

#define SENSERESISTOR 30
#define CurrentFactor  (48210/SENSERESISTOR)
// LSB=11.77uV/R= ~48210/R/4096 - convert to mA
#define ChargeCountFactor  (27443/SENSERESISTOR)
// LSB=6.7uVh/R ~27443/R/4096 - converter to mAh
#define VoltageFactor 9994

BatteryGauge::BatteryReading BatteryGauge::read() {
    registers_t registers;

    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, sizeof(registers));

    for (size_t i = 0; i < sizeof(registers); ++i) {
        registers.bytes[i] = Wire.read();
    }

    data16_t data;

    data.bytes[0] = registers.current_low;
    data.bytes[1] = registers.current_high;

    auto current = (float)(data.u16 & 0x3fff) * STC3100_CURRENT_LSB / STC3100_CURRENT_RSENSE;
    auto current_s16 = (((uint32_t)data.u16) * CurrentFactor) >> 12;

    data.bytes[0] = registers.voltage_low;
    data.bytes[1] = registers.voltage_high;

    auto voltage = (float)(data.u16) * STC3100_VOLTAGE_LSB;
    auto voltage_s16 = (((uint32_t)data.u16) * VoltageFactor) >> 12;

    data.bytes[0] = registers.charge_low;
    data.bytes[1] = registers.charge_high;

    auto charge = (float)(data.u16) * STC3100_CHARGE_LSB / STC3100_CHARGE_RSENSE;
    auto charge_s16 = (((uint32_t)data.u16) * ChargeCountFactor) >> 12;

    data.bytes[0] = registers.counter_low;
    data.bytes[1] = registers.counter_high;

    auto counter = data.u16;
    auto counter_s16 = (data.u16);

    return {
        voltage,
        current,
        charge,
        counter,

        (int16_t)voltage_s16,
        (int16_t)current_s16,
        (int16_t)charge_s16,
        (int16_t)counter_s16
    };
}

}
