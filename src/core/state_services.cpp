#include <Arduino.h>

#include "state_services.h"

#include "leds.h"
#include "watchdog.h"
#include "power_management.h"
#include "status.h"
#include "gps.h"
#include "user_button.h"
#include "scheduler.h"
#include "performance.h"

namespace fk {

template<>
MainServices *MainServicesState::services_{ nullptr };

template<>
WifiServices *WifiServicesState::services_{ nullptr };

bool MainServices::alive() {
    watchdog->task();
    power->task();
    status->task();
    gps->read();
    button->task();
    return leds->task();
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
