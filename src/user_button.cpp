#include "user_button.h"

#include "tuning.h"
#include "hardware.h"
#include "leds.h"
#include "file_system.h"

namespace fk {

UserButton::UserButton(Leds &leds, FileSystem &fileSystem) : Task("Button"), leds_(&leds), fileSystem_(&fileSystem) {
}

void UserButton::enqueued() {
    pinMode(Hardware::USER_BUTTON_PIN, INPUT);
}

TaskEval UserButton::task() {
    auto value = !(digitalRead(Hardware::USER_BUTTON_PIN) > 0);
    auto changed = value != pressed_;
    if (changed) {
        pressed_ = value;

        if (!pressed_) {
            if (millis() - changedAt_ > ButtonLongPressDuration) {
                log("Restart!");
                fileSystem_->flush();
                NVIC_SystemReset();
            }

            changedAt_ = 0;
        }
        else {
            changedAt_ = millis();
        }
    }
    else if (pressed_) {
        if (millis() - changedAt_ > ButtonLongPressDuration) {
            leds_->restarting();
        }
    }

    return TaskEval::idle();
}

}
