#include "download_file_task.h"
#include "file_system.h"
#include "wifi_client.h"

namespace fk {

DownloadFileTask::DownloadFileTask(FileSystem &fileSystem, CoreState &state, AppReplyMessage &reply,
                                   MessageBuffer &buffer, WifiConnection &connection, FileCopySettings &settings) :
    Task("DownloadFile"), fileSystem(&fileSystem), state(&state), reply(&reply), buffer(&buffer), connection(&connection), settings(settings) {
}

void DownloadFileTask::enqueued() {
    bytesCopied = 0;
    began = false;
    if (!fileSystem->beginFileCopy(settings)) {
        log("Failed to open file");
    }
}

bool DownloadFileTask::writeHeader(uint32_t total) {
    auto &fileCopy = fileSystem->files().fileCopy();

    reply->clear();
    reply->m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
    reply->m().fileData.size = total;
    reply->m().fileData.version = fileCopy.version();
    reply->m().fileData.id = (uint32_t)settings.file;

    // TODO: Is using the buffer here necessary?
    if (!buffer->write(*reply)) {
        log("Error writing reply");
        return false;
    }

    auto size = buffer->position();
    if (buffer->write() != size) {
        log("Error sending buffer");
        return false;
    }

    log("Wrote header prefix (%d bytes)", size);

    return true;
}

uint32_t DownloadFileTask::calculateTotalSize(uint32_t metadataSize) {
    auto metadataOnly = settings.flags & fk_app_DownloadFlags_DOWNLOAD_FLAG_METADATA_ONLY;
    auto prependMetadata = settings.flags & fk_app_DownloadFlags_DOWNLOAD_FLAG_METADATA_PREPEND;
    auto size = 0;

    if (prependMetadata || metadataOnly) {
        size += metadataSize;
    }

    if (!metadataOnly) {
        auto &fileCopy = fileSystem->files().fileCopy();
        size += fileCopy.remaining();
    }

    return size;
}

TaskEval DownloadFileTask::task() {
    auto &fileCopy = fileSystem->files().fileCopy();

    auto metadataOnly = settings.flags & fk_app_DownloadFlags_DOWNLOAD_FLAG_METADATA_ONLY;
    auto prependMetadata = settings.flags & fk_app_DownloadFlags_DOWNLOAD_FLAG_METADATA_PREPEND;

    if (!began) {
        StaticPool<128> pool{"DataPool"};
        DataRecordMetadataMessage drm{ *state, pool };
        uint8_t metadataBuffer[drm.calculateSize()];
        auto stream = pb_ostream_from_buffer(metadataBuffer, sizeof(metadataBuffer));
        if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
            log("Error encoding data file record (%d bytes)", sizeof(metadataBuffer));
            return TaskEval::error();
        }
        auto metadataSize = stream.bytes_written;

        // TODO: Would be nice to be able to skip this if we weren't sending.
        // Seems like calculateSize always gets us the right value.
        if (!writeHeader(calculateTotalSize(metadataSize))) {
            return TaskEval::error();
        }

        if (prependMetadata || metadataOnly) {
            auto &wcl = connection->getClient();
            wcl.write(metadataBuffer, metadataSize);

            log("Wrote metadata prefix (%d bytes)", metadataSize);

            bytesCopied += metadataSize;
        }

        began = true;
    }

    if (!metadataOnly && !fileCopy.isFinished()) {
        auto writer = WifiWriter{ connection->getClient() };
        if (!fileCopy.copy(writer)) {
            log("Error copying");
            return TaskEval::error();
        }
    }
    else {
        log("Done");
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
