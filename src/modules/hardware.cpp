#include <Arduino.h>
#include <SPI.h>

#include "debug.h"
#include "board.h"
#include "hardware.h"
#include "platform.h"

namespace fk {

extern Board board;

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

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
    if (spi_power_.task()) {
        Logger::info("SPI off");
        board.disable_spi();
    }
}

void ModuleHardware::flash_take() {
    if (spi_power_.take()) {
        Logger::info("SPI on");
        board.enable_spi();
    }
}

void ModuleHardware::flash_release() {
    if (spi_power_.release()) {
        Logger::info("SPI off");
        board.disable_spi();
    }
}

FlashEnabler::FlashEnabler(ModuleHardware *hardware) : hardware_(hardware) {
    hardware_->flash_take();
}

FlashEnabler::~FlashEnabler() {
    hardware_->flash_release();
}

bool PowerSwitch::take() {
    auto taken = false;

    if (refs_ == 0) {
        taken = true;
        time_on_ = fk_uptime();
        time_off_ = 0;
    }

    refs_++;

    return taken;
}

bool PowerSwitch::release() {
    bool released = false;

    if (refs_ > 0) {
        refs_--;

        if (refs_ == 0) {
            if (minimum_on_ > 0 && minimum_on_ > fk_uptime() - time_on_) {
                time_off_ = time_on_ + minimum_on_;
            }
            else {
                released = true;
                time_on_ = 0;
                time_off_ = 0;
            }
        }
    }

    return released;
}

bool PowerSwitch::task() {
    bool released = false;

    if (refs_ == 0) {
        if (time_off_ > 0) {
            if (fk_uptime() > time_off_) {
                time_on_ = 0;
                time_off_ = 0;
                released = true;
            }
        }
    }

    return released;
}

}
