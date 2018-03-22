#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <Arduino.h>

#ifdef FK_NATURALIST
#define FK_RTC_PCF8523
#define FK_WIFI_AP_DISABLE
#define FK_HARDWARE_SERIAL2_ENABLE
#else
#define FK_HARDWARE_WIRE11AND13_ENABLE
#endif

namespace fk {

#ifdef FK_HARDWARE_SERIAL2_ENABLE
extern Uart Serial2;
#endif

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
    #ifdef FK_NATURALIST
    static constexpr uint8_t FLASH_PIN_CS = PIN_LED_TXL;
    #else
    static constexpr uint8_t FLASH_PIN_CS = 4;
    #endif

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

};

}

#endif
