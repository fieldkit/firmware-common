#include <Arduino.h>
#include <SPI.h>

#include "hardware.h"
#include "platform.h"
#include "debug.h"

namespace fk {

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

FlashEnabler::FlashEnabler(ModuleHardware *hardware) : hardware_(hardware) {
    hardware_->flash_take();
}

FlashEnabler::~FlashEnabler() {
    hardware_->flash_release();
}

void ModuleHardware::task() {
    if (flash_refs_ == 0) {
        if (flash_enable > 0) {
            if (flash_off_ > 0) {
                if (fk_uptime() > flash_off_) {
                    Logger::info("Powering down");
                    disable_spi();
                    digitalWrite(flash_enable, LOW);
                    flash_off_ = 0;
                }
            }
        }
    }
}

void ModuleHardware::flash_take() {
    flash_refs_++;

    if (flash_enable > 0) {
        Logger::info("Powering up");
        enable_spi();
        digitalWrite(flash_enable, HIGH);
        flash_on_ = fk_uptime();
    }
}

void ModuleHardware::flash_release() {
    if (flash_refs_ > 0) {
        flash_refs_--;

        if (flash_refs_ == 0) {
            if (flash_enable > 0) {
                if (minimum_enable_time_ > 0) {
                    flash_off_ = flash_on_ + minimum_enable_time_;
                }
                else {
                    Logger::info("Powering down");
                    disable_spi();
                    digitalWrite(flash_enable, LOW);
                }
            }
        }
    }
}

void ModuleHardware::disable_spi() {
    uint8_t pins[] = {
        flash,
        SPI_PIN_MISO, SPI_PIN_MOSI, SPI_PIN_SCK,
    };
    for (auto pin : pins) {
        pinMode(pin, INPUT);
    }
}

void ModuleHardware::enable_spi() {
    uint8_t pins[] = { flash };
    for (auto pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    SPI.begin();
}

}
