#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <Arduino.h>

#if defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

#define FK_RTC_PCF8523
#define FK_HARDWARE_SERIAL2_ENABLE

/**
 * The FkNat board uses the pin usually mapepd to PIN_LED_TXL for the CS on
 * Serial Flash. Unfortunately, the USB driver code uses this pin to blinky
 * blink while sending and receiving. So as long as that's defined we can't
 * use the serial flash.
 */
#ifdef PIN_LED_TXL
#pragma message "Disabling serial flash due to collision with PIN_LED_TXL. Please remove this from variant.h."
#define FK_DISABLE_FLASH
#endif // PIN_LED_TXL

#else // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

#define FK_HARDWARE_WIRE11AND13_ENABLE

#endif // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

namespace fk {

#ifdef FK_HARDWARE_SERIAL2_ENABLE
extern Uart Serial2;
#endif

class Hardware {
public:
    // TODO: This could be done much better.
    #if defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)
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

class SerialPort {
private:
    Uart *uart;

public:
    SerialPort(Uart &uart) : uart(&uart) {
    }

public:
    void begin(int32_t baud);
    bool available();
    int32_t read();
    void println(const char *str);

};

}

#endif
