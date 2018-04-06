#include "two_wire_task.h"
#include "tuning.h"

namespace fk {

TwoWireTask::TwoWireTask(const char *name, TwoWireBus &bus, Reader &outgoing, Writer &incoming, uint8_t address) :
    Task(name), bus(&bus), outgoing(&outgoing), incoming(&incoming), address(address) {
}

TwoWireTask::TwoWireTask(const char *name, TwoWireBus &bus, Writer &incoming, uint8_t address) :
    Task(name), bus(&bus), outgoing(nullptr), incoming(&incoming), address(address) {
}

void TwoWireTask::enqueued() {
    dieAt = 0;
    checkAt = 0;
    bytesReceived = 0;
    doneAt = 0;
    expectedReplies = 1;
    bytesSent = 0;
    if (outgoing == nullptr) {
        checkAt = millis() + 200;
    }
}

TaskEval TwoWireTask::task() {
    if (checkAt > 0 && millis() < checkAt) {
        return TaskEval::idle();
    }

    if (dieAt == 0) {
        if (outgoing != nullptr) {
            return send();
        }
        else {
            dieAt = millis() + MaximumTwoWireReply;
            checkAt = millis() + 100;
            return TaskEval::idle();
        }
    }

    if (millis() > dieAt) {
        log("Error: No reply in time.");
        return TaskEval::error();
    }

    return receive();
}

TaskEval TwoWireTask::send() {
    uint8_t buffer[SERIAL_BUFFER_SIZE];
    auto bytes = outgoing->read(buffer, sizeof(buffer));
    if (bytes < 0) {
        return TaskEval::done();
    }
    if (bytes == 0) {
        return TaskEval::idle();
    }

    bytesSent += bytes;

    log("Sending %lu bytes to module (%lu)", bytes, bytesSent);

    if (!bus->send(address, buffer, bytes)) {
        log("Error: Unable to send.");
        return TaskEval::error();
    }

    dieAt = millis() + MaximumTwoWireReply;
    // They won't be ready yet, check back soon, though.
    checkAt = millis() + 100;

    return TaskEval::idle();
}

TaskEval TwoWireTask::receive() {
    if (expectedReplies > 0) {
        uint8_t buffer[SERIAL_BUFFER_SIZE];
        bytesReceived = bus->receive(address, buffer, sizeof(buffer));
        if (bytesReceived == 0) {
            log("Error: Empty reply.");
            return TaskEval::error();
        }

        log("Received %d (%d) bytes from module", bytesReceived, sizeof(buffer));

        auto wrote = incoming->write(buffer, bytesReceived);
        if (wrote != (int32_t)bytesReceived) {
            log("Error: Out of buffer space (%lu != %d)", wrote, bytesReceived);
            // return TaskEval::error();
            // return TaskEval::idle();
        }
        else {
            expectedReplies--;
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
