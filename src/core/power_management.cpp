#include "debug.h"
#include "tuning.h"
#include "power_management.h"
#include "core_fsm.h"
#include "configuration.h"

#if defined(FK_FUEL_GAUGE_MAX1704)
#include <FuelGauge.h>
#else
#include "battery_gauge.h"
#endif

namespace fk {

constexpr const char Log[] = "PM";

using Logger = SimpleLog<Log>;

Power::Power(CoreState &state) : state_(&state) {
}

void Power::setup() {
    #if defined(FK_FUEL_GAUGE_MAX1704)
    Logger::info("No fuel gauge!");
    fk_assert(false);
    #else
    Logger::info("Initializing fuel gauge...");
    BatteryGauge gauge;
    #endif

    available_ = gauge.enable();

    Logger::info("Initialized");
}

struct MapEntry {
    float p;
    float mv;
};

constexpr MapEntry naive_map[] = {
    { 0.0f, 3.5f  * 1000.0f },
    { 0.1f, 3.57f * 1000.0f },
    { 0.2f, 3.64f * 1000.0f },
    { 0.3f, 3.71f * 1000.0f },
    { 0.4f, 3.78f * 1000.0f },
    { 0.5f, 3.85f * 1000.0f },
    { 0.6f, 3.92f * 1000.0f },
    { 0.7f, 3.99f * 1000.0f },
    { 0.8f, 4.06f * 1000.0f },
    { 0.9f, 4.13f * 1000.0f },
    { 1.0f, 4.2f  * 1000.0f },
};

void Power::task() {
    if (fk_uptime() > query_time_) {
        #if defined(FK_FUEL_GAUGE_MAX1704)
        fk_assert(false);
        #else
        BatteryGauge gauge;
        #endif

        auto first_time = query_time_ == 0;

        if (available_) {
            auto reading = gauge.read();
            auto percentage = 0.0f;
            auto attached = false;

            Logger::info("Updating (%0.3fmv, I = %0.3fmA, %0.3fmAh, %d)", reading.voltage, reading.ma, reading.coulombs, reading.counter);

            for (size_t i = 0; i < sizeof(naive_map) / sizeof(float[2]); ++i) {
                if (reading.voltage < naive_map[i].mv) {
                    // If our voltage is below 3.5f then there's just no battery.
                    if (i == 0) {
                        attached = false;
                        percentage = 0.0f;
                        break;
                    }

                    auto start_mv = naive_map[i - 1].mv;
                    auto end_mv = naive_map[i].mv;
                    auto start_p = naive_map[i - 1].p;
                    auto end_p = naive_map[i].p;
                    auto f = (reading.voltage - start_mv) / (end_mv - start_mv);

                    percentage = (((end_p - start_p) * f) + start_p) * 100.0f;

                    break;
                }

                percentage = naive_map[i].p * 100.0f;
            }

            auto low = percentage < BatteryLowPowerSleepThreshold;
            auto ok = percentage > BatteryLowPowerResumeThreshold;
            auto update = BatteryStatus{
                attached,
                low,
                ok,
                reading.voltage,
                reading.coulombs,
                first_time ? (0.0f) : (reading.coulombs - status_.coulombs),
                percentage,
                reading.ma
            };

            status_ = update;

            state_->updateBattery(status_);

            if (fk_uptime() - last_alert_ > PowerManagementAlertInterval) {
                if (status_.low) {
                    if (configuration.sleeping.low_power) {
                        send_event(LowPowerEvent{ });
                    }
                }
                last_alert_ = fk_uptime();
            }
        }
        else {
            Logger::info("Unavailable");
        }

        query_time_ = fk_uptime() + PowerManagementQueryInterval;
    }
}

}
