#include <Arduino.h>

#include "user_button.h"

#include "core_fsm.h"

#include "tuning.h"
#include "hardware.h"
#include "leds.h"
#include "file_system.h"

namespace fk {

static UserButton *global_button{ nullptr };

static void irq_button() {
    if (global_button != nullptr) {
        global_button->handler();
    }
}

UserButton::UserButton(Leds &leds) : Task("Button"), leds_(&leds) {
    global_button = this;
}

void UserButton::enqueued() {
    // Setup hardware, interested in changes.
    pinMode(Hardware::USER_BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(Hardware::USER_BUTTON_PIN), irq_button, CHANGE);

    // Check the button, just in case they were holding the thing when we started.
    handler();
}

void UserButton::handler() {
    auto value = !(digitalRead(Hardware::USER_BUTTON_PIN) > 0);
    auto changed = value != pressed_;
    if (changed) {
        pressed_ = value;

        if (pressed_) {
            pending_ = PendingButtonEvent::None;
            changedAt_ = fk_uptime();
            notified_ = 0;
        }
        else {
            changedAt_ = 0;
        }
    }
}

TaskEval UserButton::task() {
    if (wasPressed_ != pressed_) {
        log("Change");
        if (pressed_) {
            leds_->notifyButtonPressed();
        }
        else {
            leds_->notifyButtonReleased();

            switch (pending_) {
            case PendingButtonEvent::Long: {
                send_event(LongButtonPressEvent{ });
                break;
            }
            case PendingButtonEvent::Short: {
                send_event(ShortButtonPressEvent{ });
                break;
            }
            case PendingButtonEvent::None: {
                send_event(MinorButtonPressEvent{ });
                break;
            }
            }

            pending_ = PendingButtonEvent::None;
        }

        wasPressed_ = pressed_;
    }

    if (pressed_) {
        if (fk_uptime() - notified_ > 250) {
            send_event(UserButtonEvent{ });
            notified_ = fk_uptime();
        }

        if (fk_uptime() - changedAt_ > ButtonLongPressDuration) {
            if (pending_ != PendingButtonEvent::Long) {
                log("Long");
                leds_->notifyButtonLong();
                pending_ = PendingButtonEvent::Long;
            }
        }
        else if (fk_uptime() - changedAt_ > ButtonShortPressDuration) {
            if (pending_ != PendingButtonEvent::Short) {
                log("Short");
                leds_->notifyButtonShort();
                pending_ = PendingButtonEvent::Short;
            }
        }
    }

    return TaskEval::idle();
}

}
