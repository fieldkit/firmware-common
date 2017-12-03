#include "two_wire_task.h"

namespace fk {

constexpr char QueryCapabilities::Name[];
constexpr char QuerySensorCapabilities::Name[];
constexpr char BeginTakeReading::Name[];
constexpr char QueryReadingStatus::Name[];

TaskEval TwoWireTask::task() {
    if (checkAt > 0 && millis() < checkAt) {
        return TaskEval::idle();
    }

    MessageBuffer buffer;

    if (dieAt == 0) {
        buffer.write(query);
        if (!buffer.send(address)) {
            return TaskEval::error();
        }

        dieAt = millis() + 1000;
        // They won't be ready yet, check back soon, though.
        checkAt = millis() + 100;
        return TaskEval::idle();
    }
    else if (millis() > dieAt) {
        return TaskEval::error();
    }

    if (!buffer.receive(address)) {
        return TaskEval::error();
    }
    if (!buffer.read(reply)) {
        return TaskEval::error();
    }

    if (reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
        checkAt = millis() + 200;
        return TaskEval::idle();
    }

    return TaskEval::done();
}

}
