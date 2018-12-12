#include <Arduino.h>
#include <SPI.h>

#include "debug.h"
#include "board.h"
#include "hardware.h"
#include "platform.h"

namespace fk {

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

extern Board board;

FlashEnabler ModuleHardware::enable_flash() {
    return { this };
}

bool ModuleHardware::has_flash() {
    return board.flash_cs() > 0;
}

uint8_t ModuleHardware::flash_cs() {
    return board.flash_cs();
}

void ModuleHardware::task() {
    if (flash_refs_ == 0) {
        if (flash_off_ > 0) {
            if (fk_uptime() > flash_off_) {
                Logger::info("Powering down");
                board.disable_spi();
                flash_on_ = 0;
                flash_off_ = 0;
            }
        }
    }
}

void ModuleHardware::flash_take() {
    if (flash_refs_ == 0) {
        Logger::info("Powering up");
        board.enable_spi();
        flash_on_ = fk_uptime();
        flash_off_ = 0;
    }

    flash_refs_++;
}

void ModuleHardware::flash_release() {
    if (flash_refs_ > 0) {
        flash_refs_--;

        if (flash_refs_ == 0) {
            if (500 > fk_uptime() - flash_on_) {
                flash_off_ = flash_on_ + 500;
            }
            else {
                Logger::info("Powering down");
                board.disable_spi();
                flash_on_ = 0;
                flash_off_ = 0;
            }
        }
    }
}

FlashEnabler::FlashEnabler(ModuleHardware *hardware) : hardware_(hardware) {
    hardware_->flash_take();
}

FlashEnabler::~FlashEnabler() {
    hardware_->flash_release();
}

}
