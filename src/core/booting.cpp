#include "booting.h"
#include "hardware.h"
#include "leds.h"
#include "watchdog.h"
#include "power_management.h"
#include "user_button.h"
#include "factory_reset_check.h"

namespace fk {

void Booting::task() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    // This only works if I do this before we initialize the WDT, for some
    // reason. Not a huge priority to fix but I'd like to understand why
    // eventually.
    // 44100
    // NOTE: FkNaturalist Specific
    // fk_assert(AudioInI2S.begin(8000, 32));

    services().leds->setup();
    services().watchdog->setup();
    services().bus->begin(400000);
    services().power->setup();
    services().button->enqueued();

    transit<FactoryResetCheck>();
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
