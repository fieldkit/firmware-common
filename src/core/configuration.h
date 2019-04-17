#ifndef FK_CORE_CONFIGURATION_H_INCLUDED
#define FK_CORE_CONFIGURATION_H_INCLUDED

#include <lwcron/lwcron.h>

#include "common_configuration.h"

#if !defined(FK_API_BASE)
#define FK_API_BASE "http://api.fkdev.org"
#endif

namespace fk {

namespace defaults {

constexpr uint32_t WifiInactivityTimeout = 2 * Minutes;
constexpr uint32_t WifiCaptivitiyTimeout = 10 * Seconds;

}

struct Configuration {
    struct Wifi {
        /**
         *
         */
        uint32_t inactivity_time{ defaults::WifiInactivityTimeout };

        /**
         *
         */
        uint32_t captivity_time{ defaults::WifiCaptivitiyTimeout };

        /**
         *
         */
        const char stream_url[128] = FK_API_BASE "/messages/ingestion/stream";

        /**
         *
         */
        const char firmware_url[128] = FK_API_BASE "/devices/%s/%s/firmware";
    };

    struct Gps {
        /**
         *
         */
        uint32_t on_duration  = 20 * Minutes;

        /**
         *
         */
        uint32_t status_interval  = 30 * Seconds;

        /**
         *
         */
        uint32_t clear_interval  = 5 * Minutes;

        /**
         *
         */
        uint8_t required_satellites = 1;

        /**
         *
         */
        bool echo{ false };

        /**
         *
         */
        #if defined(FK_GPS_FIXED_STATION)
        bool station_fixed{ true };
        #else
        bool station_fixed{ false };
        #endif
    };

    struct Schedule {
        #if defined(FK_PROFILE_AMAZON)
        lwcron::CronSpec readings{ lwcron::CronSpec::specific(0, 0) };
        #else
        lwcron::CronSpec readings{ lwcron::CronSpec::specific(0) };
        #endif

        #if defined(FK_PROFILE_AMAZON)
        lwcron::CronSpec wifi{ lwcron::CronSpec::specific(0, 10) };
        #else
        lwcron::CronSpec wifi{ lwcron::CronSpec::everyFiveMinutes() };
        #endif

        lwcron::CronSpec lora;
    };

    struct Sleeping {
        #if defined(FK_ENABLE_LOW_POWER_SLEEP)
        bool low_power{ true };
        #else
        bool low_power{ false };
        #endif

        #if defined(FK_ENABLE_DEEP_SLEEP)
        bool deep{ true };
        #else
        bool deep{ false };
        #endif
    };

    struct Logging {
        bool discovery{ true };
    };

    Wifi wifi;
    Gps gps;
    Schedule schedule;
    Sleeping sleeping;
    CommonConfiguration common;
    Logging logging;

    #if defined(FK_NATURALIST)
    const char *display_name = "FieldKit Naturalist";
    const char *module_name = "fk-naturalist";
    #else
    const char *display_name = "FieldKit Device";
    const char *module_name = "fk-core";
    #endif

    uint32_t no_modules_sleep{ 30 * Minutes };
    uint32_t no_modules_rescan{ 1 * Minutes };
    uint32_t modules_ready_time{ 7 * Seconds };
};

/**
 *
 */
extern const Configuration configuration;

}

#endif
