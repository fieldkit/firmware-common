#include <functional>

#include "transmit_file.h"
#include "restart_wizard.h"
#include "file_cursors.h"

namespace fk {

FileCopierSample::FileCopierSample(FileSystem &fileSystem, CoreState &state) : Task("FileCopy"), fileSystem_(&fileSystem), state_(&state) {
}

void FileCopierSample::enqueued() {
    if (!fileSystem_->beginFileCopy({ FileNumber::StartupLog })) {
    }
}

TaskEval FileCopierSample::task() {
    auto &fileCopy = fileSystem_->files().fileCopy();

    if (!fileCopy.isFinished()) {
        auto writer = lws::NullWriter{};
        if (!fileCopy.copy(writer)) {
            return TaskEval::done();
        }
    }
    else {
        return TaskEval::done();
    }

    return TaskEval::idle();
}

TransmitFileTask::TransmitFileTask(FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config, FileCopySettings settings) :
    Task("TransmitFileTask"), fileSystem(&fileSystem), state(&state), wifi(&wifi), config(&config), settings(settings) {
}

void TransmitFileTask::enqueued() {
    tries = 0;
    connected = false;
    waitingSince = millis();
}

TaskEval TransmitFileTask::task() {
    if (!connected) {
        FileCursorManager fcm(*fileSystem);
        auto position = fcm.lookup(settings.file);

        if (!fileSystem->beginFileCopy(FileCopySettings{ settings.file, (uint32_t)position, 0 })) {
            return TaskEval::error();
        }

        auto &fileCopy = fileSystem->files().fileCopy();

        if (fileCopy.remaining() == 0) {
            log("Empty: (%lu) %d -> %d", (uint32_t)position, fileCopy.tell(), fileCopy.size());
            return TaskEval::done();
        }
        else {
            log("Uploading: %d", fileCopy.remaining());
        }

        waitingSince = 0;

        return openConnection();
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    auto &fileCopy = fileSystem->files().fileCopy();

    auto status = parser.getStatusCode();
    if (!wcl.connected() || status > 0) {
        wcl.flush();
        wcl.stop();
        if (status == 200) {
            if (!fileCopy.isFinished()) {
                log("Unfinished success (status = %d)", status);
            }
            else {
                log("Success (status = %d)", status);
            }

            FileCursorManager fcm(*fileSystem);
            if (!fcm.save(settings.file, fileCopy.tell())) {
                log("Failed to save cursor: %d", sizeof(FileCursors));
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

    if (!fileCopy.isFinished()) {
        auto writer = WifiWriter{ wcl };
        if (!fileCopy.copy(writer)) {
            return TaskEval::error();
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
                return TaskEval::error();
            }

            auto &fileCopy = fileSystem->files().fileCopy();
            auto bufferSize = stream.bytes_written;
            auto fileSize = fileCopy.size() - fileCopy.tell();
            auto transmitting = fileSize + bufferSize;

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

            log("Sending %d + %d = %d bytes...", fileSize, bufferSize, transmitting);
            connected = true;
            wcl.write(buffer, bufferSize);
        } else {
            log("Not connected!");
            return TaskEval::error();
        }
    } else {
        return TaskEval::error();
    }

    return TaskEval::idle();
}

}
