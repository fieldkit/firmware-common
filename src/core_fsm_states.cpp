#include "core_fsm_states.h"
#include "wifi_states.h"

#include "attached_devices.h"
#include "scheduler.h"

#include <Arduino.h>

namespace fk {

template<>
MainServices *MainServicesState::services_{ nullptr };
template<>
WifiServices *WifiServicesState::services_{ nullptr };

class Booting;
class Initializing;
class Idle;
class ScanAttachedDevices;

class Booting : public CoreDevice {
public:
    void entry() override {
        CoreDevice::entry();
        transit<Initializing>();
    }
};

class Initializing : public CoreDevice {
public:
    void entry() override {
        CoreDevice::entry();
    }

    void task() override {
        transit<ScanAttachedDevices>();
    }
};

class Sleep : public CoreDevice {
public:
    void entry() override {
        CoreDevice::entry();
        log("Sleep");
    }

    void task() override {
        for (auto i = 0; i < 8; ++i) {
            delay(1000);
        }
        transit<Idle>();
    }
};

class ScanAttachedDevices : public MainServicesState {
public:
    void entry() override {
        MainServicesState::entry();
        log("ScanAttachedDevices");
    }

    void task() override {
        auto e = services().attachedDevices->task();
        if (e.isDone()) {
            transit<WifiStartup>();
        }
    }
};

void Idle::entry() {
    MainServicesState::entry();
    log("Idle");
}

void Idle::task() {
    if (millis() - checked_ > 500) {
        auto nextTask = services().scheduler->getNextTask();
        if (nextTask.seconds > 10) {
            transit<Sleep>();
        }
        checked_ = millis();
    }
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
