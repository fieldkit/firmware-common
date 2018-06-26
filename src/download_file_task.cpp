#include "download_file_task.h"

namespace fk {

DownloadFileTask::DownloadFileTask(CoreState &state, AppReplyMessage &reply, MessageBuffer &buffer) :
    Task("DownloadFile"), state(&state), reply(&reply), buffer(&buffer) {
}

void DownloadFileTask::enqueued() {
    // iterator.beginning();
    bytesCopied = 0;
}

TaskEval DownloadFileTask::task() {
    /*
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

        pb_data_t dataData = {
            .length = bufferSize,
            .buffer = metadataBuffer,
        };

        reply->clear();
        reply->m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
        reply->m().fileData.data.funcs.encode = pb_encode_data;
        reply->m().fileData.data.arg = (void *)&dataData;
        reply->m().fileData.size = 0; // iterator.size();

        if (!buffer->write(*reply)) {
            log("Error writing reply");
            return TaskEval::error();
        }

        auto size = buffer->position();
        if (buffer->write() != size) {
            log("Error sending buffer");
            return TaskEval::error();
        }

        log("Wrote metadata prefix (%d bytes)", bufferSize);
    }

    auto data = DataBlock{ }; // iterator.move();
    if (data && data.size > 0) {
        pb_data_t dataData = {
            .length = data.size,
            .buffer = data.ptr,
        };

        pb_data_t tokenData = {
            .length = 0, // sizeof(fkfs_iterator_token_t),
            .buffer = nullptr, // &iterator.resumeToken(),
        };

        reply->clear();
        reply->m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
        reply->m().fileData.data.funcs.encode = pb_encode_data;
        reply->m().fileData.data.arg = (void *)&dataData;
        reply->m().fileData.token.funcs.encode = pb_encode_data;
        reply->m().fileData.token.arg = (void *)&tokenData;
        reply->m().fileData.size = 0; // iterator.size();

        bytesCopied += data.size;

        if (!buffer->write(*reply)) {
            log("Error writing reply");
            return TaskEval::error();
        }

        auto size = buffer->position();
        if (buffer->write() != size) {
            log("Error sending buffer");
            return TaskEval::error();
        }
    }

    if (iterator.isFinished()) {
        return TaskEval::done();
    }
    */

    return TaskEval::idle();
}

}
