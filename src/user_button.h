#ifndef FK_USER_BUTTON_H_INCLUDED
#define FK_USER_BUTTON_H_INCLUDED

#include "active_object.h"

namespace fk {

class Leds;
class FileSystem;

class UserButton : public Task {
private:
    Leds *leds_;
    FileSystem *fileSystem_;
    bool pressed_{ false };
    uint32_t changedAt_{ 0 };

public:
    UserButton(Leds &leds, FileSystem &fileSystem);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
