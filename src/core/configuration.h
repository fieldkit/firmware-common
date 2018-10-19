#ifndef FK_CORE_CONFIGURATION_H_INCLUDED
#define FK_CORE_CONFIGURATION_H_INCLUDED

#include <lwcron/lwcron.h>

#include "common_configuration.h"

#if !defined(FK_API_BASE)
#define FK_API_BASE "http://api.fkdev.org"
#endif

namespace fk {

namespace defaults {

constexpr uint32_t WifiInactivityTimeout = 1 * Minutes;
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
        lwcron::CronSpec readings{ lwcron::CronSpec::everyFiveMinutes() };
        #endif

        #if defined(FK_WIFI_STARTUP_ONLY)
        lwcron::CronSpec wifi;
        #else
        lwcron::CronSpec wifi{ lwcron::CronSpec::specific(0, 10) };
        #endif

        lwcron::CronSpec lora;
    };

    Wifi wifi;
    Gps gps;
    Schedule schedule;
    CommonConfiguration common;

    #if defined(FK_NATURALIST)
    const char *display_name = "FieldKit Naturalist";
    const char *module_name = "fk-naturalist";
    #else
    const char *display_name = "FieldKit Device";
    const char *module_name = "fk-core";
    #endif

    #if defined(FK_CORE_REQUIRE_MODULES)
    uint32_t no_modules_sleep{ 30 * Minutes };
    #else
    uint32_t no_modules_sleep{ 0 };
    #endif
};

/**
 *
 */
extern const Configuration configuration;

}

#endif
