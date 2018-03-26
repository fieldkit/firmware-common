#include "transmit_readings.h"
#include "restart_wizard.h"

namespace fk {

constexpr uint32_t MaximumUpload = 1024 * 1024;

TransmitAllQueuedReadings::TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config, TwoWireBus &bus, Pool &pool) :
    Task("TransmitAllQueued"), iterator(fs, file), state(&state), wifi(&wifi), config(&config), bus(&bus), pool(&pool) {
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
        if (state->isBusy() || state->isReadingInProgress()) {
            log("We're busy, skipping.");
            return TaskEval::done();
        }

        iterator.reopen(state->getTransmissionCursor());

        if (iterator.size() > MaximumUpload) {
            log("Skipping, upload too large at %d bytes.", iterator.size());
            iterator.end();
            state->setTransmissionCursor(iterator.resumeToken());
            return TaskEval::done();
        }

        return openConnection();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    auto status = parser.getStatusCode();
    if (!wcl.connected() || status > 0) {
        wcl.stop();
        state->setBusy(false);
        wifi->setBusy(false);
        if (status == 200) {
            if (!iterator.isFinished()) {
                log("Unfinished success (status = %d)", status);
            }
            else {
                log("Success (status = %d)", status);

                if (state->shouldWipeAfterUpload()) {
                    log("Truncating data!");
                    iterator.truncateFile();
                }
            }
            state->setTransmissionCursor(iterator.resumeToken());
        }
        else {
            log("Failed (status = %d)", status);
        }
        return TaskEval::done();
    }

    if (!iterator.isFinished()) {
        auto data = iterator.move();
        if (data && data.size > 0) {
            wcl.write((uint8_t *)data.ptr, data.size);
        }
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

        if (cachedDns.cached(parsed.server) && wcl.connect(cachedDns.ip(), parsed.port)) {
            DataRecordMetadataMessage drm{ *state, *pool };
            uint8_t buffer[drm.calculateSize()];
            auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
            if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
                log("Error encoding data file record (%d bytes)", sizeof(buffer));
                wcl.stop();
                state->setBusy(false);
                wifi->setBusy(false);
                return TaskEval::error();
            }

            auto bufferSize = stream.bytes_written;
            auto transmitting = iterator.size() + bufferSize;

            HttpResponseWriter httpWriter(wcl);
            httpWriter.writeHeaders(parsed, "application/vnd.fk.data+binary", transmitting);

            log("Sending %d bytes...", transmitting);
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
