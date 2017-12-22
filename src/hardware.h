#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

namespace fk {

class Hardware {
public:
    static constexpr uint8_t WIFI_PIN_CS = 7;
    static constexpr uint8_t WIFI_PIN_IRQ = 9;
    static constexpr uint8_t WIFI_PIN_RST = 10;
    static constexpr uint8_t WIFI_PIN_EN = 11;
    static constexpr uint8_t WIFI_PIN_WAKE = 8;

    static constexpr uint8_t RFM95_PIN_CS = 5;
    static constexpr uint8_t RFM95_PIN_RST = 4;
    static constexpr uint8_t RFM95_PIN_INT = 3;

    static constexpr uint8_t SD_PIN_CS = 12;
    static constexpr uint8_t FLASH_PIN_CS = 4;

    // static constexpr uint8_t SD_PIN_CS = 10;
    // static constexpr uint8_t WIFI_PIN_CS = 8;
    // static constexpr uint8_t WIFI_PIN_IRQ = 7;
    // static constexpr uint8_t WIFI_PIN_RST = 4;
    // static constexpr uint8_t WIFI_PIN_EN = 2;
    // static constexpr uint8_t WIFI_PIN_WAKE = 0xff;
};

}

#endif
