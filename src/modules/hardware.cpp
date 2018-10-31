#include "hardware.h"
#include "platform.h"
#include <Arduino.h>

namespace fk {

FlashEnabler::FlashEnabler(ModuleHardware *hardware) : hardware_(hardware) {
    hardware_->flash_take();
}

FlashEnabler::~FlashEnabler() {
    hardware_->flash_release();
}

void ModuleHardware::task() {
    if (flash_enable > 0) {
        if (flash_off_ > 0) {
            if (fk_uptime() > flash_off_) {
                digitalWrite(flash_enable, LOW);
                flash_off_ = 0;
            }
        }
    }
}

void ModuleHardware::flash_take() {
    if (flash_enable > 0) {
        digitalWrite(flash_enable, HIGH);
        flash_on_ = fk_uptime();
    }
}

void ModuleHardware::flash_release() {
    if (flash_enable > 0) {
        if (minimum_enable_time_ > 0) {
            flash_off_ = flash_on_ + minimum_enable_time_;
        }
        else {
            digitalWrite(flash_enable, LOW);
        }
    }
}

}
