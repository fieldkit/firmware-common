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
    Status(CoreState &state, TwoWireBus &bus) : state(&state), bus(&bus) {
    }

public:
    void idle() override {
        if (millis() - lastTick > 5000) {
            DeviceId deviceId{ *bus };
            IpAddress4 ip{ state->getStatus().ip };
            debugfpln("Status", "Status (%.2f%% / %.2fmv) (%lu free) (%s) (%s)",
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
    power.setup();

    delay(10);

    fk_assert(fileSystem.setup());

    watchdog.started();

    bus.begin();

    DeviceId deviceId{ bus };
    state.setDeviceId(deviceId.toString());

    clock.begin();

    state.started();
}

void CoreModule::run() {
    fk::SimpleNTP ntp(clock);
    Status status{ state, bus };

    wifi.begin();

    scheduler.push(ntp);

    while (true) {
        status.tick();
        leds.tick();
        power.tick();
        watchdog.tick();
        attachedDevices.tick();
        if (attachedDevices.isIdle()) {
            liveData.tick();
            if (liveData.isIdle()) {
                scheduler.tick();
            }
            wifi.tick();
            discovery.tick();
        }
    }
}

}

