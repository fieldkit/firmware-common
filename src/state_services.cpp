#include <Arduino.h>

#include "state_services.h"

#include "leds.h"
#include "watchdog.h"
#include "power_management.h"
#include "status.h"
#include "gps.h"
#include "user_button.h"
#include "scheduler.h"

namespace fk {

template<>
MainServices *MainServicesState::services_{ nullptr };

template<>
WifiServices *WifiServicesState::services_{ nullptr };

void MainServices::alive() {
    leds->task();
    watchdog->task();
    power->task();
    status->task();
    gps->task();
    button->task();
}

DateTime MainServices::scheduledTasks() {
    auto now = clock.now();
    auto lwcTime = lwcron::DateTime{ now.unixtime() };
    scheduler->check(lwcTime);
    return now;
}

void MainServices::clear() {
    pool->clear();
}

}
