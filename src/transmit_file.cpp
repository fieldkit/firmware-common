#include "transmit_file.h"
#include "restart_wizard.h"

#include <functional>

namespace fk {

FileCopierSample::FileCopierSample(FileSystem &fileSystem, CoreState &state) : Task("FileCopy"), fileSystem(&fileSystem), state(&state) {
}

void FileCopierSample::enqueued() {
}

TaskEval FileCopierSample::task() {
    auto &fileReader = fileSystem->files().reader();

    if (!fileReader.isOpen()) {
        return TaskEval::done();
    }

    if (!fileReader.isFinished()) {
        auto writer = lws::NullWriter{};
        auto copied = streamCopier.copy(fileReader, writer);
        if (copied != lws::Stream::EOS) {
            trace("Copied: %lu %d / %d", copied, fileReader.tell(), fileReader.size());
        }
        else {
            trace("Copied: EOF %d / %d", fileReader.tell(), fileReader.size());
        }
    }
    else {
        return TaskEval::done();
    }

    return TaskEval::idle();
}

TransmitFileTask::TransmitFileTask(FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config) :
    Task("TransmitFileTask"), fileSystem(&fileSystem), state(&state), wifi(&wifi), config(&config) {
}

void TransmitFileTask::enqueued() {
    tries = 0;
    connected = false;
    waitingSince = millis();
}

TaskEval TransmitFileTask::task() {
    if (!wifi->possiblyOnline()) {
        log("Wifi disabled or using local AP");
        return TaskEval::done();
    }

    if (!connected) {
        if (state->isBusy() || state->isReadingInProgress()) {
            if (millis() - waitingSince > WifiTransmitBusyWaitMax) {
                log("We're busy, skipping.");
                return TaskEval::done();
            }

            return TaskEval::busy();
        }

        if (!fileSystem->openForReading(4)) {
            return TaskEval::error();
        }

        auto &fileReader = fileSystem->files().reader();
        fileReader.open();
        trace("Opened: %d / %d", fileReader.tell(), fileReader.size());

        fileReader.open();
        streamCopier.restart();
        waitingSince = 0;

        return openConnection();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    auto &fileReader = fileSystem->files().reader();

    auto status = parser.getStatusCode();
    if (!wcl.connected() || status > 0) {
        wcl.flush();
        wcl.stop();
        state->setBusy(false);
        wifi->setBusy(false);
        if (status == 200) {
            if (!fileReader.isFinished()) {
                log("Unfinished success (status = %d)", status);
            }
            else {
                log("Success (status = %d)", status);
            }
        }
        else {
            tries++;
            log("Failed (status = %d) tries(%d / %d)", status, tries, (uint8_t)WifiTransmitFileMaximumTries);
            if (tries >= WifiTransmitFileMaximumTries) {
                return TaskEval::error();
            }
            else {
                connected = false;
                waitingSince = millis();
                return TaskEval::busy();
            }
        }
        return TaskEval::done();
    }

    if (!fileReader.isFinished()) {
        auto writer = WifiWriter{ wcl };
        auto copied = streamCopier.copy(fileReader, writer);
        if (copied != lws::Stream::EOS) {
            trace("Copied: %lu %d / %d", copied, fileReader.tell(), fileReader.size());
        }
        else {
            trace("Copied: EOF %d / %d", fileReader.tell(), fileReader.size());
        }
    }
    else {
        if (waitingSince == 0) {
            waitingSince = millis();
        }
        if (millis() - waitingSince > WifiTransmitBusyWaitMax) {
            log("No response after (%lu).", WifiTransmitBusyWaitMax);
            wcl.flush();
            wcl.stop();
        }
    }

    return TaskEval::idle();
}

TaskEval TransmitFileTask::openConnection() {
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
            StaticPool<128> pool{"DataPool"};
            DataRecordMetadataMessage drm{ *state, pool };
            uint8_t buffer[drm.calculateSize()];
            auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
            if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
                log("Error encoding data file record (%d bytes)", sizeof(buffer));
                wcl.flush();
                wcl.stop();
                state->setBusy(false);
                wifi->setBusy(false);
                return TaskEval::error();
            }

            auto &fileReader = fileSystem->files().reader();
            auto bufferSize = stream.bytes_written;
            auto transmitting = fileReader.size() + bufferSize;

            HttpResponseWriter httpWriter(wcl);
            OutgoingHttpHeaders headers{
                    "application/vnd.fk.data+binary",
                    transmitting,
                    firmware_version_get(),
                    firmware_build_get(),
                    deviceId.toString(),
                    0
            };
            httpWriter.writeHeaders(parsed, headers);

            log("Sending %d + %d = %d bytes...", fileReader.size(), bufferSize, transmitting);
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

}
