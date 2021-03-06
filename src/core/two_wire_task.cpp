#include "two_wire_task.h"
#include "tuning.h"
#include "performance.h"

namespace fk {

ReplyConfig ReplyConfig::Default{ };

ReplyConfig ReplyConfig::NoReply{ false };

/**
 * This is high because we retry fast and some operations, like DATA_PREPARE can
 * take a few seconds. Most other operations will require a smaller number of
 * retries, so maybe this is kind of ugly.
 */
ReplyConfig ReplyConfig::Long{ ReplyConfig::TwoWireLongTimeout };

TwoWireTask::TwoWireTask(const char *name, TwoWireBus &bus, lws::Reader &outgoing, lws::Writer &incoming, uint8_t address, ReplyConfig replyConfig) :
    Task(name), bus(&bus), outgoing(&outgoing), incoming(&incoming), address(address), replyConfig(replyConfig) {
}

TwoWireTask::TwoWireTask(const char *name, TwoWireBus &bus, lws::Writer &incoming, uint8_t address, ReplyConfig replyConfig) :
    Task(name), bus(&bus), outgoing(nullptr), incoming(&incoming), address(address), replyConfig(replyConfig) {
}

void TwoWireTask::enqueued() {
    dieAt = 0;
    checkAt = 0;
    bytesReceived = 0;
    doneAt = 0;
    bytesSent = 0;
    repliesRemaining = replyConfig.expected_replies;
    // If this is nullptr then we've already issued the query and are in the
    // middle of a retry because we got told they were busy. So use the busy delay.
    if (outgoing == nullptr) {
        checkAt = fk_uptime() + replyConfig.busy_delay;
    }
    bus->begin();
}

TaskEval TwoWireTask::task() {
    if (checkAt > 0 && fk_uptime() < checkAt) {
        return TaskEval::idle();
    }

    if (dieAt == 0) {
        if (outgoing != nullptr) {
            return send();
        }
        else {
            if (repliesRemaining > 0) {
                dieAt = fk_uptime() + replyConfig.reply_timeout;
                checkAt = fk_uptime() + replyConfig.reply_delay;
                return TaskEval::idle();
            }
            else {
                return TaskEval::done();
            }
        }
    }

    if (fk_uptime() > dieAt) {
        log("Error: No reply in time.");
        return TaskEval::error();
    }

    return receive();
}

TaskEval TwoWireTask::send() {
    uint8_t buffer[SERIAL_BUFFER_SIZE - 1];
    auto bytes = outgoing->read(buffer, sizeof(buffer));
    if (bytes < 0) {
        return TaskEval::done();
    }
    if (bytes == 0) {
        return TaskEval::idle();
    }

    bytesSent += bytes;

    #ifdef FK_TWO_WIRE_LOGGING_VERBOSE
    trace("Sending %lu bytes to module (%lu)", bytes, bytesSent);
    #endif

    if (!bus->send(address, buffer, bytes)) {
        log("Error: Unable to send.");
        return TaskEval::error();
    }

    dieAt = fk_uptime() + replyConfig.reply_timeout;

    if (repliesRemaining == 0) {
        return TaskEval::idle();
    }

    // They won't be ready yet, check back soon, though.
    checkAt = fk_uptime() + replyConfig.reply_delay;

    return TaskEval::idle();
}

TaskEval TwoWireTask::receive() {
    if (repliesRemaining > 0) {
        uint8_t buffer[SERIAL_BUFFER_SIZE - 1];
        bytesReceived = bus->receive(address, buffer, sizeof(buffer));
        if (bytesReceived == 0) {
            log("Error: Empty reply.");
            return TaskEval::error();
        }

        #ifdef FK_TWO_WIRE_LOGGING_VERBOSE
        trace("Received %d (%d) bytes from module", bytesReceived, sizeof(buffer));
        #endif

        auto wrote = incoming->write(buffer, bytesReceived);
        if (wrote != (int32_t)bytesReceived) {
            log("Error: Out of buffer space (%lu != %d)", wrote, bytesReceived);
        }
        else {
            repliesRemaining--;
        }
    }

    if (outgoing == nullptr) {
        // We don't have a reader to indicate we're done, so just be done.
        return TaskEval::done();
    }

    dieAt = 0;
    return TaskEval::idle();
}

}
