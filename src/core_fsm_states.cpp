#include "core_fsm_states.h"
#include "wifi_states.h"

#include "attached_devices.h"
#include "scheduler.h"
#include "leds.h"

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
    const char *name() const override {
        return "Booting";
    }

public:
    void task() override {
        transit<Initializing>();
    }
};

class Initializing : public CoreDevice {
public:
    const char *name() const override {
        return "Initializing";
    }

public:
    void task() override {
        #if defined(FK_NATURALIST)
        transit<WifiStartup>();
        #else
        transit<ScanAttachedDevices>();
        #endif
    }
};

class ScanAttachedDevices : public MainServicesState {
public:
    const char *name() const override {
        return "ScanAttachedDevices";
    }

public:
    void task() override {
        uint8_t addresses[4]{ 7, 8, 9, 0 };

        AttachedDevices attachedDevices{
            *services().state,
            *services().leds,
            *services().moduleCommunications,
            addresses,
        };

        while (simple_task_run(attachedDevices)) {
            // TODO: Should never take so long we need the watchdog.
            services().leds->task();
            services().moduleCommunications->task();
        }

        transit<WifiStartup>();
    }
};

class Sleep : public CoreDevice {
public:
    const char *name() const override {
        return "Sleep";
    }

public:
    void task() override {
        for (auto i = 0; i < 4; ++i) {
            delay(1000);
        }
        transit<Idle>();
    }
};

void Idle::entry() {
    MainServicesState::entry();
    if (began_ == 0) {
        began_ = fk_uptime();
    }
}

void Idle::react(SchedulerEvent const &se) {
    if (se.deferred) {
        warn("Scheduler Event!");
        transit(se.deferred);
    }
}

void Idle::task() {
    services().scheduler->task();

    if (fk_uptime() - checked_ > 500) {
        auto nextTask = services().scheduler->getNextTask();
        if (nextTask.seconds > 10) {
            transit<Sleep>();
        }
        checked_ = fk_uptime();
    }

    if (fk_uptime() - began_ > 60 * 1000) {
        began_ = 0;
        transit<WifiStartup>();
    }
}

void BeginGatherReadings::task() {
    back();
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
