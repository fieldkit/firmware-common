#include "core_fsm_states.h"
#include "wifi_states.h"

#include "attached_devices.h"
#include "scheduler.h"
#include "leds.h"
#include "watchdog.h"
#include "status.h"
#include "power_management.h"
#include "user_button.h"

#include "file_system.h"
#include "wifi_states.h"
#include "gps.h"

#include <Arduino.h>

namespace fk {

template<>
MainServices *StateWithContext<MainServices>::services_{ nullptr };

template<>
WifiServices *WifiServicesState::services_{ nullptr };

class Booting;
class Initializing;
class CheckPower;
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
        transit<CheckPower>();
    }
};

class LowPowerSleep : public MainServicesState {public:
public:
    void task() {
        transit<RebootDevice>();
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
private:
    uint32_t maximum_{ 0 };

public:
    Sleep() {
    }

    Sleep(uint32_t maximum) : maximum_(maximum) {
    }

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

void MainServicesState::alive() {
    services().leds->task();
    services().watchdog->task();
    services().button->task();
    services().power->task();
    services().status->task();
}

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
    if (fk_uptime() - checked_ > 500) {
        auto nextTask = services().scheduler->getNextTask();
        if (nextTask.seconds > 10) {
            transit_into<Sleep>(nextTask.seconds - 5); // We're greater than 10.
            return;
        }
        checked_ = fk_uptime();
    }

    services().scheduler->task();

    alive();
}

void CheckPower::task() {
    #if defined(FK_NATURALIST)
    transit<WifiStartup>();
    #else
    transit<ScanAttachedDevices>();
    #endif
}

void RebootDevice::task() {
    services().fileSystem->flush();
    NVIC_SystemReset();
}

class TakeReadings : public MainServicesState {
public:
    const char *name() const override {
        return "TakeReadings";
    }

public:
    void task() override {
        resume();
    }
};

class TakeGpsReading : public MainServicesState {
public:
    const char *name() const override {
        return "TakeGpsReading";
    }

public:
    void task() override {
        SerialPort gpsSerial{ Hardware::gpsUart };
        ReadGps gps{ *services().state, gpsSerial };

        gps.enqueued();

        while (elapsed() < GpsFixAttemptInterval) {
            if (!simple_task_run(gps)) {
                break;
            }

            alive();
        }
        transit<TakeReadings>();
    }
};

void BeginGatherReadings::task() {
    resume_at_back();

    transit<TakeGpsReading>();
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
