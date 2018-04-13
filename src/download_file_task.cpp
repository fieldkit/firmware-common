#include "download_file_task.h"

namespace fk {

DownloadFileTask::DownloadFileTask(fkfs_t *fs, uint8_t file, fkfs_iterator_token_t *resumeToken, AppReplyMessage &reply, MessageBuffer &buffer) :
    Task("DownloadFile"), reply(&reply), buffer(&buffer), iterator(*fs, file, resumeToken) {
}

void DownloadFileTask::enqueued() {
    iterator.beginning();
}

TaskEval DownloadFileTask::task() {
    auto data = iterator.move();
    if (data && data.size > 0) {
        pb_data_t dataData = {
            .length = data.size,
            .buffer = data.ptr,
        };

        pb_data_t tokenData = {
            .length = sizeof(fkfs_iterator_token_t),
            .buffer = &iterator.resumeToken(),
        };

        reply->clear();
        reply->m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
        reply->m().fileData.data.funcs.encode = pb_encode_data;
        reply->m().fileData.data.arg = (void *)&dataData;
        reply->m().fileData.token.funcs.encode = pb_encode_data;
        reply->m().fileData.token.arg = (void *)&tokenData;
        reply->m().fileData.size = iterator.size();

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

    return TaskEval::idle();
}

}
