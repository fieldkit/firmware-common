#ifndef FK_CORE_CONFIGURATION_H_INCLUDED
#define FK_CORE_CONFIGURATION_H_INCLUDED

#include "tuning.h"

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
    };

    struct Gps {
        /**
         *
         */
        uint32_t status_interval  = 30 * Seconds;

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

    struct Leds {
        /**
         *
         */
        uint32_t disable_time{ LedsDisableAfter };
    };

    Wifi wifi;
    Gps gps;
    Leds leds;
};

/**
 *
 */
extern const Configuration configuration;

}

#endif
