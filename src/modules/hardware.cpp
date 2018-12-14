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

SpiEnabler ModuleHardware::enable_spi() {
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

void ModuleHardware::spi_take() {
    if (spi_power_.take()) {
        Logger::info("SPI on");
        board.enable_spi();
    }
}

void ModuleHardware::spi_release() {
    if (spi_power_.release()) {
        Logger::info("SPI off");
        board.disable_spi();
    }
}

SpiEnabler::SpiEnabler(ModuleHardware *hardware) : hardware_(hardware) {
    hardware_->spi_take();
}

SpiEnabler::~SpiEnabler() {
    hardware_->spi_release();
}

bool PowerSwitch::has_been_on_for(uint32_t time) const {
    return on() && fk_uptime() - time_on_ > time;
}

bool PowerSwitch::on() const {
    return time_on_ > 0;
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
        if (refs_ == 1) {
            if (time_off_ > 0) {
                if (fk_uptime() > time_off_) {
                    released = true;
                }
            }
            else if (minimum_on_ > 0) {
                if (time_off_ == 0) {
                    time_off_ = time_on_ + minimum_on_;
                }
            }
            else {
                released = true;
            }
        }
        else {
            refs_--;
        }
    }

    if (released) {
        time_on_ = 0;
        time_off_ = 0;
        refs_ = 0;
    }

    return released;
}

bool PowerSwitch::task() {
    bool released = false;

    if (time_off_ > 0 && fk_uptime() > time_off_) {
        time_on_ = 0;
        time_off_ = 0;
        refs_ = 0;
        released = true;
    }

    return released;
}

bool PowerSwitch::touch() {
    time_off_ = fk_uptime() + minimum_on_;

    return false;
}

}
