#ifndef FK_USER_BUTTON_H_INCLUDED
#define FK_USER_BUTTON_H_INCLUDED

#include "task.h"

namespace fk {

class Leds;
class FileSystem;

class UserButton : public Task {
private:
    enum class PendingButtonEvent {
        None,
        Wakeup,
        Reboot
    };

    Leds *leds_;
    bool pressed_{ false };
    uint32_t changedAt_{ 0 };
    uint32_t notified_{ 0 };
    PendingButtonEvent pending_{ PendingButtonEvent::None };

public:
    UserButton(Leds &leds);

public:
    void handler();
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
