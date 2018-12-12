#include "board_definition.h"

#include "hardware.h"

namespace fk {

Board board{
    {
        Hardware::PERIPHERALS_ENABLE_PIN,
        Hardware::FLASH_PIN_CS,
        {
            Hardware::FLASH_PIN_CS,
            Hardware::WIFI_PIN_CS,
            Hardware::RFM95_PIN_CS,
            Hardware::SD_PIN_CS,
        },
        {
            Hardware::PERIPHERALS_ENABLE_PIN,
            Hardware::GPS_ENABLE_PIN,
            0,
            0,
        }
    }
};

}
