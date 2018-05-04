#include "transmit_file.h"
#include "restart_wizard.h"

#include <functional>

namespace fk {

TransmitAllFilesTask::TransmitAllFilesTask(TaskQueue &taskQueue, FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config)
    : Task("TransmitAllFilesTask"), taskQueue(&taskQueue),
      queue{ make_array( std::make_tuple(std::ref(fileSystem), (uint8_t)0, std::ref(state), std::ref(wifi), std::ref(config)),
                         std::make_tuple(std::ref(fileSystem), (uint8_t)1, std::ref(state), std::ref(wifi), std::ref(config)) ) } {
}

TaskEval TransmitAllFilesTask::task() {
    taskQueue->append(queue);
    return TaskEval::done();
}

TransmitFileTask::TransmitFileTask(FileSystem &fileSystem, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config) :
    Task("TransmitFileTask"), fileReader(fileSystem, file), state(&state), wifi(&wifi), config(&config) {
}

void TransmitFileTask::enqueued() {
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
            if (millis() - waitingSince > TransmitBusyWaitMax) {
                log("We're busy, skipping.");
                return TaskEval::done();
            }

            return TaskEval::busy();
        }

        fileReader.open(state->getCursor(fileReader.fileNumber()));

        if (fileReader.size() > MaximumUpload) {
            log("Skipping, upload too large at %lu bytes.", fileReader.size());
            fileReader.end();
            state->saveCursor(fileReader.resumeToken());
            return TaskEval::done();
        }

        waitingSince = 0;

        return openConnection();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

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

                if (state->shouldWipeAfterUpload()) {
                    log("Truncating data!");
                    fileReader.truncate();
                }
            }
            state->saveCursor(fileReader.resumeToken());
        }
        else {
            log("Failed (status = %d)", status);
            enqueued();
            return TaskEval::busy();
        }
        return TaskEval::done();
    }

    if (!fileReader.isFinished()) {
        auto writer = WifiWriter{ wcl };
        streamCopier.copy(fileReader, writer);
    }
    else {
        if (waitingSince == 0) {
            waitingSince = millis();
        }
        if (millis() - waitingSince > TransmitBusyWaitMax) {
            log("No response after (%lu).", TransmitBusyWaitMax);
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

            auto bufferSize = stream.bytes_written;
            auto transmitting = fileReader.size() + bufferSize;

            HttpResponseWriter httpWriter(wcl);
            httpWriter.writeHeaders(parsed, "application/vnd.fk.data+binary", transmitting);

            log("Sending %lu + %d = %lu bytes...", fileReader.size(), bufferSize, transmitting);
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
