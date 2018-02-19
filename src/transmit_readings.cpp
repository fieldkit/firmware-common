#include "transmit_readings.h"
#include "restart_wizard.h"

namespace fk {

TransmitAllQueuedReadings::TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config, TwoWireBus &bus, Pool &pool) :
    ActiveObject("TransmitAllQueued"), iterator(fs, file), state(&state), wifi(&wifi), config(&config), bus(&bus), pool(&pool) {
}

void TransmitAllQueuedReadings::enqueued() {
    connected = false;
}

TaskEval TransmitAllQueuedReadings::task() {
    if (!wifi->possiblyOnline()) {
        log("Wifi disabled or using local AP");
        return TaskEval::done();
    }

    if (!connected) {
        return openConnection();
    }

    if (!wcl.connected()) {
        log("Disconnected (statusCode=%d)", parser.getStatusCode());
        wcl.stop();
        state->setBusy(false);
        wifi->setBusy(false);
        return TaskEval::done();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    auto data = iterator.move();
    if (data && data.size > 0) {
        wcl.write((uint8_t *)data.ptr, data.size);
    }

    if (iterator.isFinished()) {
        log("Finished (free = %lu)", fk_free_memory());
        state->setTransmissionCursor(iterator.resumeToken());
        wcl.stop();
        state->setBusy(false);
        wifi->setBusy(false);
        log("Done, disconnecting (statusCode=%d)", parser.getStatusCode());
        return TaskEval::done();
    }

    return TaskEval::idle();
}

TaskEval TransmitAllQueuedReadings::openConnection() {
    const auto length = strlen(config->streamUrl) + 1;
    char urlCopy[length];
    strncpy(urlCopy, config->streamUrl, length);
    Url parsed(urlCopy);

    state->setBusy(true);
    wifi->setBusy(true);

    parser.begin();

    if (parsed.server != nullptr && parsed.path != nullptr) {
        log("Connecting: '%s:%d' / '%s'", parsed.server, parsed.port, parsed.path);

        if (config->cachedDns.cached(parsed.server) && wcl.connect(config->cachedDns.ip(), parsed.port)) {
            iterator.reopen(state->getTransmissionCursor());

            DataRecordMetadataMessage drm{ *state, *pool };
            uint8_t buffer[drm.calculateSize()];
            auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
            if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
                log("Error encoding data file record (%d bytes)", sizeof(buffer));
                state->setBusy(false);
                wifi->setBusy(false);
                return TaskEval::error();
            }

            auto bufferSize = stream.bytes_written;
            auto transmitting = iterator.size() + bufferSize;

            HttpResponseWriter httpWriter(wcl);
            httpWriter.writeHeaders(parsed, "application/vnd.fk.data+binary", transmitting);

            log("Connected, transmitting %d...", transmitting);
            connected = true;
            wcl.write(buffer, bufferSize);
        } else {
            log("Not connected!");
            state->setBusy(false);
            wifi->setBusy(false);
            return TaskEval::error();
        }
    } else {
        state->setBusy(false);
        wifi->setBusy(false);
        return TaskEval::error();
    }

    return TaskEval::idle();
}

void TransmitAllQueuedReadings::parseRecord(DataBlock &data) {
    DataRecordMessage message{ *pool };

    auto stream = pb_istream_from_buffer((uint8_t *)data.ptr, data.size);
    if (!pb_decode_delimited(&stream, fk_data_DataRecord_fields, message.forDecode())) {
        log("error: Unable to decode message (size %d)", data.size);
        return;
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

}
