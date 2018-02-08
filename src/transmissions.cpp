#include "transmissions.h"

namespace fk {

TransmissionTask::TransmissionTask(const char *name) : ActiveObject(name) {
}

void TransmissionTask::prepare(MessageBuilder &mb) {
    builder = &mb;
}

void TransmissionTask::write(Print &stream) {
    if (!builder->write(stream)) {
        log("Error writing message.");
    }
}

const char *TransmissionTask::getContentType() {
    return builder->getContentType();
}

TransmitAllQueuedReadings::TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Pool &pool) : ActiveObject("TransmitAllQueued"), iterator(fs, file), state(&state), pool(&pool) {
}

void TransmitAllQueuedReadings::enqueued() {
    iterator.reopen(state->getTransmissionCursor());
}

TaskEval TransmitAllQueuedReadings::task() {
    auto data = iterator.move();
    if (data && data.size > 0) {
        DataRecordMessage message{ *pool };

        auto stream = pb_istream_from_buffer((uint8_t *)data.ptr, data.size);
        if (!pb_decode_delimited(&stream, fk_data_DataRecord_fields, message.forDecode())) {
            log("error: Unable to decode message (size %d)", data.size);
            return TaskEval::idle();
        }

        if (message.m().loggedReading.version > 0) {
            auto &lr = message.m().loggedReading;
            auto &location = lr.location;
            auto &reading = lr.reading;

            if (false) {
                if (lr.reading.time > 0) {
                    log("message: location: %f %f: %lu %lu = %f", location.longitude, location.latitude, (uint32_t)reading.time, reading.sensor, reading.value);
                }
                else {
                    log("message: location: %f %f", location.longitude, location.latitude);
                }
            }
        }
    }

    if (iterator.isFinished()) {
        state->setTransmissionCursor(iterator.resumeToken());
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
