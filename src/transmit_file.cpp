#include <functional>

#include "transmit_file.h"
#include "restart_wizard.h"
#include "file_cursors.h"
#include "http_response_writer.h"

namespace fk {

FileCopierSample::FileCopierSample(FileSystem &fileSystem, CoreState &state) : Task("FileCopy"), fileSystem_(&fileSystem), state_(&state) {
}

void FileCopierSample::enqueued() {
    if (!fileSystem_->beginFileCopy({ FileNumber::Data })) {
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
    copyFinishedAt = 0;
}

void TransmitFileTask::fileCopyTick() {
    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }
}

TaskEval TransmitFileTask::task() {
    if (!connected) {
        if (!openFile()) {
            return TaskEval::done();
        }

        return openConnection();
    }

    fileCopyTick();

    auto &fileCopy = fileSystem->files().fileCopy();

    if (!fileCopy.isFinished()) {
        auto writer = WifiWriter{ wcl };
        if (!fileCopy.copy(writer)) {
            return TaskEval::error();
        }
    }

    if (fileCopy.isFinished()) {
        if (copyFinishedAt == 0) {
            copyFinishedAt = fk_uptime();
        }
        if (fk_uptime() - copyFinishedAt > WifiTransmitBusyWaitMax) {
            log("No response after (%lu).", WifiTransmitBusyWaitMax);
            wcl.flush();
            wcl.stop();
        }
    }

    auto status = parser.status_code();
    if (!wcl.connected() || status > 0) {
        auto afterClosed = fk_uptime() - copyFinishedAt;

        wcl.flush();
        wcl.stop();
        if (status == 200) {
            auto position = fileCopy.tell();

            if (!fileCopy.isFinished()) {
                log("Unfinished success (status = %d) (%lums)", status, afterClosed);
            }
            else {
                log("Success (status = %d) (%lums) (position = %d)", status, afterClosed, position);
            }

            FileCursorManager fcm(*fileSystem);
            if (!fcm.save(settings.file, position)) {
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
                copyFinishedAt = 0;
                return TaskEval::busy();
            }
        }
        return TaskEval::done();
    }

    return TaskEval::idle();
}

bool TransmitFileTask::openFile() {
    FileCursorManager fcm(*fileSystem);
    auto position = fcm.lookup(settings.file);

    if (!fileSystem->beginFileCopy(FileCopySettings{ settings.file, (uint32_t)position, 0 })) {
        log("Error opening file");
        return false;
    }

    auto &fileCopy = fileSystem->files().fileCopy();
    if (fileCopy.remaining() == 0) {
        log("Empty: (%lu) %d -> %d", (uint32_t)position, fileCopy.tell(), fileCopy.size());
        return false;
    }
    else {
        log("Uploading: %d", fileCopy.remaining());
    }
    return true;
}

TaskEval TransmitFileTask::openConnection() {
    Url parsed(config->streamUrl);

    copyFinishedAt = 0;
    parser.begin();

    if (parsed.server != nullptr && parsed.path != nullptr) {
        log("Connecting: '%s:%d' / '%s'", parsed.server, parsed.port, parsed.path);

        if (cachedDns.cached(parsed.server) && wcl.connect(cachedDns.ip(), parsed.port)) {
            if (!writeBeginning(parsed)) {
                return TaskEval::error();
            }
        } else {
            log("Not connected!");
            return TaskEval::error();
        }
    } else {
        return TaskEval::error();
    }

    return TaskEval::idle();
}

bool TransmitFileTask::writeBeginning(Url &parsed) {
    EmptyPool pool;
    DataRecordMetadataMessage drm{ *state, pool };
    uint8_t buffer[drm.calculateSize()];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
        log("Error encoding data file record (%d bytes)", sizeof(buffer));
        wcl.flush();
        wcl.stop();
        return false;
    }

    auto &fileCopy = fileSystem->files().fileCopy();
    auto bufferSize = stream.bytes_written;
    auto fileSize = fileCopy.remaining();
    auto transmitting = fileSize + bufferSize;

    HttpHeadersWriter httpWriter(&wcl);
    OutgoingHttpHeaders headers{
        "application/vnd.fk.data+binary",
        transmitting,
        firmware_version_get(),
        firmware_build_get(),
        firmware_compiled_get(),
        deviceId.toString(),
        (uint8_t)settings.file
    };
    httpWriter.writeHeaders(parsed, "POST", headers);

    log("Sending %d + %d = %d bytes...", fileSize, bufferSize, transmitting);
    connected = true;
    wcl.write(buffer, bufferSize);

    return true;
}

}
