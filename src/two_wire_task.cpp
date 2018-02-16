#include "two_wire_task.h"

namespace fk {

constexpr char QueryCapabilities::Name[];
constexpr char QuerySensorCapabilities::Name[];
constexpr char BeginTakeReading::Name[];
constexpr char QueryReadingStatus::Name[];
constexpr char CustomModuleQueryTask::Name[];

TaskEval TwoWireTask::task() {
    if (checkAt > 0 && millis() < checkAt) {
        return TaskEval::idle();
    }

    TwoWireMessageBuffer buffer{ *bus };

    if (dieAt == 0) {
        buffer.write(query);
        if (!buffer.send(address)) {
            log("Error: Unable to send.");
            return TaskEval::error();
        }

        dieAt = millis() + 1000;
        // They won't be ready yet, check back soon, though.
        checkAt = millis() + 100;
        return TaskEval::idle();
    } else if (millis() > dieAt) {
        log("Error: No reply in time.");
        return TaskEval::error();
    }

    if (!buffer.receive(address)) {
        log("Error: Unable to receive.");
        return TaskEval::error();
    }
    if (!buffer.read(reply)) {
        log("Error: Unable to read reply.");
        return TaskEval::error();
    }

    if (reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
        checkAt = millis() + 200;
        return TaskEval::idle();
    }

    return TaskEval::done();
}

CustomModuleQueryTask &CustomModuleQueryTask::ready(AppQueryMessage &newAppQuery) {
    appQuery = &newAppQuery;
    query.m().custom.message.arg = appQuery->m().module.message.arg;
    return *this;
}

}
