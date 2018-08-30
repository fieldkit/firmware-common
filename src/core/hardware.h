#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <cinttypes>

#include "platform.h"
#include "serial_port.h"

#if defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

/**
 * The FkNat board uses the pin usually mapepd to PIN_LED_TXL for the CS on
 * Serial Flash. Unfortunately, the USB driver code uses this pin to blinky
 * blink while sending and receiving. So as long as that's defined we can't
 * use the serial flash.
 */
#ifdef PIN_LED_TXL
#pragma message "Disabling serial flash due to collision with PIN_LED_TXL. Please remove this from variant.h."
#endif // PIN_LED_TXL

#endif // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

namespace fk {

class Hardware {
public:
    // TODO: This could be done much better.
    #if defined(FK_CORE_GENERATION_2)
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 16;
    static constexpr uint8_t WIFI_PIN_RST = 15;
    static constexpr uint8_t WIFI_PIN_EN = 38;
    static constexpr uint8_t WIFI_PIN_WAKE = 8;

    static constexpr uint8_t RFM95_PIN_CS = 5;
    static constexpr uint8_t RFM95_PIN_RESET = 3;
    static constexpr uint8_t RFM95_PIN_ENABLE = 0;
    static constexpr uint8_t RFM95_PIN_D0 = 2;

    static constexpr uint8_t PIN_PERIPH_ENABLE = (25u); // PIN_LED_RXL;
    static constexpr uint8_t FLASH_PIN_CS = (26u);      // PIN_LED_TXL;
    #else
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 9;
    static constexpr uint8_t WIFI_PIN_RST = 10;
    static constexpr uint8_t WIFI_PIN_EN = 11;
    static constexpr uint8_t WIFI_PIN_WAKE = 8;

    static constexpr uint8_t RFM95_PIN_CS = 5;
    static constexpr uint8_t RFM95_PIN_RESET = 3;
    static constexpr uint8_t RFM95_PIN_ENABLE = 0;
    static constexpr uint8_t RFM95_PIN_D0 = 2;

    static constexpr uint8_t FLASH_PIN_CS = 4;
    #endif

    static constexpr uint8_t USER_BUTTON_PIN = 6;

    static constexpr uint8_t SD_PIN_CS = 12;

    static Uart &gpsUart;
};

}

#endif
