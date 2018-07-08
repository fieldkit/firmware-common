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
    if (!fileSystem->beginFileCopy(settings)) {
        log("Failed to open file");
    }
}

TaskEval DownloadFileTask::task() {
    auto &fileCopy = fileSystem->files().fileCopy();

    if (bytesCopied == 0) {
        StaticPool<128> pool{"DataPool"};
        DataRecordMetadataMessage drm{ *state, pool };
        uint8_t metadataBuffer[drm.calculateSize()];
        auto stream = pb_ostream_from_buffer(metadataBuffer, sizeof(metadataBuffer));
        if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, drm.forEncode())) {
            log("Error encoding data file record (%d bytes)", sizeof(metadataBuffer));
            return TaskEval::error();
        }
        auto bufferSize = stream.bytes_written;

        reply->clear();
        reply->m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
        reply->m().fileData.size = fileCopy.remaining() + bufferSize;

        if (!buffer->write(*reply)) {
            log("Error writing reply");
            return TaskEval::error();
        }

        auto size = buffer->position();
        if (buffer->write() != size) {
            log("Error sending buffer");
            return TaskEval::error();
        }

        auto &wcl = connection->getClient();
        wcl.write(metadataBuffer, bufferSize);

        log("Wrote metadata prefix (%d bytes)", bufferSize);

        bytesCopied += bufferSize;
    }

    if (!fileCopy.isFinished()) {
        auto writer = WifiWriter{ connection->getClient() };
        if (!fileCopy.copy(writer)) {
            log("Error");
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
