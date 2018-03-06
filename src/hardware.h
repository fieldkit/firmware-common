#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <Arduino.h>

namespace fk {

class Hardware {
public:
    // TODO: This could be done much better.
    #ifdef FK_NATURALIST
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 16;
    static constexpr uint8_t WIFI_PIN_RST = 15;
    static constexpr uint8_t WIFI_PIN_EN = 38;
    static constexpr uint8_t WIFI_PIN_WAKE = 8;
    #else
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 9;
    static constexpr uint8_t WIFI_PIN_RST = 10;
    static constexpr uint8_t WIFI_PIN_EN = 11;
    static constexpr uint8_t WIFI_PIN_WAKE = 8;
    #endif

    static constexpr uint8_t RFM95_PIN_CS = 5;
    static constexpr uint8_t RFM95_PIN_RST = 4;
    static constexpr uint8_t RFM95_PIN_INT = 3;

    static constexpr uint8_t SD_PIN_CS = 12;
    static constexpr uint8_t FLASH_PIN_CS = 4;

    static Uart &gpsUart;
};

}

#endif
