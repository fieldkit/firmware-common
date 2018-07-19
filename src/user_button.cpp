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
    pinMode(Hardware::USER_BUTTON_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(Hardware::USER_BUTTON_PIN), irq_button, CHANGE);
}

void UserButton::handler() {
    auto value = !(digitalRead(Hardware::USER_BUTTON_PIN) > 0);
    auto changed = value != pressed_;
    if (changed) {
        pressed_ = value;

        if (!pressed_) {
            if (fk_uptime() - changedAt_ > ButtonShortPressDuration) {
                pending_ = PendingButtonEvent::Wakeup;
            }

            if (fk_uptime() - changedAt_ > ButtonLongPressDuration) {
                pending_ = PendingButtonEvent::Reboot;
            }

            changedAt_ = 0;
        }
        else {
            changedAt_ = fk_uptime();
            notified_ = 0;
        }
    }
}

TaskEval UserButton::task() {
    switch (pending_) {
    case PendingButtonEvent::Reboot: {
        send_event(UserRebootEvent{ });
        pending_ = PendingButtonEvent::None;
        break;
    }
    case PendingButtonEvent::Wakeup: {
        send_event(UserWakeupEvent{ });
        pending_ = PendingButtonEvent::None;
        break;
    }
    case PendingButtonEvent::None: {
        break;
    }
    }

    if (pressed_) {
        if (fk_uptime() - notified_ > 250) {
            send_event(UserButtonEvent{ });
            notified_ = fk_uptime();
        }
    }

    return TaskEval::idle();
}

}
