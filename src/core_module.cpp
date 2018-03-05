#include "core_module.h"
#include "hardware.h"
#include "device_id.h"

namespace fk {

class Status : public ActiveObject {
private:
    uint32_t lastTick{ 0 };
    CoreState *state;
    TwoWireBus *bus;

public:
    Status(CoreState &state, TwoWireBus &bus) : ActiveObject("Status"), state(&state), bus(&bus) {
    }

public:
    void idle() override {
        if (millis() - lastTick > 5000) {
            IpAddress4 ip{ state->getStatus().ip };
            auto now = clock.now();
            debugfpln("Status", "Status %lu (%.2f%% / %.2fmv) (%lu free) (%s) (%s)", now.unixtime(),
                      state->getStatus().batteryPercentage, state->getStatus().batteryVoltage,
                      fk_free_memory(), ip.toString(), deviceId.toString());
            lastTick = millis();
        }
    }

};

CoreModule::CoreModule() {
}

void CoreModule::begin() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    leds.setup();
    watchdog.setup();
    bus.begin();
    power.setup();

    fk_assert(deviceId.initialize(bus));

    SerialNumber serialNumber;
    debugfpln("Core", "Serial(%s)", serialNumber.toString());
    debugfpln("Core", "DeviceId(%s)", deviceId.toString());
    debugfpln("Core", "Hash(%s)", firmware_version_get());

    delay(10);

    fk_assert(fileSystem.setup());

    watchdog.started();

    bus.begin();

    state.setDeviceId(deviceId.toString());

    clock.begin();

    state.started();
}

void CoreModule::run() {
    SimpleNTP ntp(clock, wifi);
    Status status{ state, bus };

    wifi.begin();

    scheduler.push(ntp);

    while (true) {
        status.task();
        leds.task();
        power.task();
        watchdog.task();
        attachedDevices.task();
        if (attachedDevices.isIdle()) {
            liveData.task();
            if (liveData.isIdle()) {
                scheduler.task();
            }
            wifi.task();
            discovery.task();
        }
    }
}

}

