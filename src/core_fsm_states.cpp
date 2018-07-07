#include "core_fsm_states.h"
#include "wifi_states.h"

#include "attached_devices.h"
#include "scheduler.h"

#include <Arduino.h>

namespace fk {

template<>
MainServices *StateWithContext<MainServices>::services_{ nullptr };

class Booting;
class Initializing;
class Idle;
class ScanAttachedDevices;

class Booting : public CoreDevice {
public:
    void entry() override {
        transit<Initializing>();
    }
};

class Initializing : public CoreDevice {
public:
    void entry() override {
    }

    void task() override {
        transit<ScanAttachedDevices>();
    }
};

class Sleep : public CoreDevice {
public:
    void entry() override {
        log("Sleep");
    }

    void task() override {
        if (Serial) {
            delay(10000);
        }
        else {
            delay(10000);
        }
        transit<Idle>();
    }
};

class ScanAttachedDevices : public StateWithContext<MainServices> {
public:
    void entry() override {
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
