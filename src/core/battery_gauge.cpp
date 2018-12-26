#include <Wire.h>
#include <Arduino.h>

#include "battery_gauge.h"

namespace fk {

// https://www.st.com/content/ccc/resource/technical/document/application_note/fa/c2/de/32/e8/c9/4e/28/CD00248578.pdf/files/CD00248578.pdf/jcr:content/translations/en.CD00248578.pdf

constexpr uint8_t STC3100_ADDRESS = 0x70;
constexpr uint8_t STC3100_REGISTER_ID = 24;
constexpr uint8_t STC3100_REGISTER_MODE = 0;
constexpr uint8_t STC3100_REGISTER_MODE_VALUE_ENABLED = 0x1 << 4;
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

bool BatteryGauge::read(uint8_t reg, data16_t &data) {
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, sizeof(data));

    for (size_t i = 0; i < sizeof(data); ++i) {
        data.bytes[i] = Wire.read();
    }

    return true;
}

bool BatteryGauge::read(uint8_t reg, data32_t &data) {
    Wire.beginTransmission(STC3100_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(STC3100_ADDRESS, sizeof(data));

    for (size_t i = 0; i < sizeof(data); ++i) {
        data.bytes[i] = Wire.read();
    }

    return true;
}

float BatteryGauge::current() {
    data16_t data;

    if (!read(STC3100_REGISTER_CURRENT, data)) {
        return 0.0f;
    }

    return (float)(data.u16) * STC3100_CURRENT_LSB / STC3100_CURRENT_RSENSE;
}

float BatteryGauge::voltage() {
    data16_t data;

    if (!read(STC3100_REGISTER_VOLTAGE, data)) {
        return 0.0f;
    }

    return (float)(data.u16) * STC3100_VOLTAGE_LSB;
}

float BatteryGauge::stateOfCharge() {
    data16_t data;

    if (!read(STC3100_REGISTER_CHARGE, data)) {
        return 0.0f;
    }

    return (float)(data.u16) * STC3100_CHARGE_LSB / STC3100_CHARGE_RSENSE;
}

}
