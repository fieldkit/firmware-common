#include "two_wire_task.h"

namespace fk {

constexpr char QueryCapabilities::Name[];
constexpr char QuerySensorCapabilities::Name[];

TaskEval &TwoWireTask::task() {
    if (checkAt > 0 && millis() < checkAt) {
        return TaskEval::Idle;
    }

    if (dieAt == 0) {
        if (!query.send(address)) {
            return TaskEval::Error;
        }

        dieAt = millis() + 1000;
        // They won't be ready yet, check back soon, though.
        checkAt = millis() + 100;
        return TaskEval::Idle;
    }
    else if (millis() > dieAt) {
        return TaskEval::Error;
    }

    if (!reply.receive(address)) {
        return TaskEval::Error;
    }

    if (reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
        checkAt = millis() + 200;
        return TaskEval::Idle;
    }

    return TaskEval::Done;
}

}
