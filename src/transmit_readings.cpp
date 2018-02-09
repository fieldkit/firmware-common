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
    if (wifi->isDisabled()) {
        log("Wifi disabled");
        return TaskEval::done();
    }

    if (WiFi.status() != WL_AP_CONNECTED && WiFi.status() != WL_CONNECTED) {
        log("Wifi disconnected");
        return TaskEval::done();
    }

    if (!connected) {
        return openConnection();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    if (!wcl.connected()) {
        wcl.stop();
        log("Disconnected (statusCode=%d)", parser.getStatusCode());
        state->setBusy(false);
        return TaskEval::done();
    }

    auto data = iterator.move();
    if (data && data.size > 0) {
        wcl.write((uint8_t *)data.ptr, data.size);
    }

    if (iterator.isFinished()) {
        log("Finishing...");
        state->setTransmissionCursor(iterator.resumeToken());
        log("Stop connection");
        wcl.stop();
        state->setBusy(false);
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

    parser.begin();

    // TODO: Verify we got good values? Though this should
    // probably have been checked before.
    if (parsed.server != nullptr && parsed.path != nullptr) {
        log("Connecting: '%s:%d' / '%s'", parsed.server, parsed.port, parsed.path);

        if ((uint32_t)config->cachedAddress == (uint32_t)0) {
            if (!WiFi.hostByName(parsed.server, config->cachedAddress)) {
                log("DNS failure on '%s'", parsed.server);
            }
        }

        // TODO: Fix blocking.
        if (config->cachedAddress != (uint32_t)0 && wcl.connect(config->cachedAddress, parsed.port)) {
            iterator.reopen(state->getTransmissionCursor());

            auto drm = DataRecordMetadataMessage{ *bus, *state, *pool };
            auto metadataSize = drm.calculateSize();
            log("Need %d more", metadataSize);

            uint8_t buffer[metadataSize + ProtoBufEncodeOverhead];
            auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
            if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
                log("Error encoding data file record (%d/%d bytes)", metadataSize, sizeof(buffer));
                state->setBusy(false);
                return TaskEval::error();
            }

            log("Connected, transmitting %d", iterator.size());
            connected = true;

            wcl.print("POST /");
            wcl.print(parsed.path);
            wcl.println(" HTTP/1.1");
            wcl.print("Host: ");
            wcl.println(parsed.server);
            wcl.print("Content-Type: ");
            wcl.println("application/vnd.fk.data+binary");
            wcl.print("Content-Length: ");
            wcl.println(iterator.size() + stream.bytes_written);
            wcl.println("Connection: close");
            wcl.println();
            wcl.write(buffer, stream.bytes_written);
        } else {
            log("Not connected!");
            state->setBusy(false);
            return TaskEval::error();
        }
    } else {
        state->setBusy(false);
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
