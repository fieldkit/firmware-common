#include <Arduino.h>

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
                    digitalWrite(flash_enable, LOW);
                }
            }
        }
    }
}

}
