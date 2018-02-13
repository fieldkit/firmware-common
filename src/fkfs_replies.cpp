#include <new>

#include "fkfs_replies.h"
#include "debug.h"
#include "utils.h"

namespace fk {

constexpr uint32_t DefaultPageSize = (size_t)(8 * 4096);

FkfsReplies::FkfsReplies(fkfs_t &fs, uint8_t dataFileId) : fs(&fs), dataFileId(dataFileId), downloadFileTask(nullptr) {
}

void FkfsReplies::queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    auto numberOfFiles = fkfs_number_of_files(fs);
    fkfs_file_info_t fkfsFiles[numberOfFiles];
    fk_app_File replyFiles[numberOfFiles];

    for (auto i = 0; i < numberOfFiles; ++i) {
        fkfs_get_file(fs, i, &fkfsFiles[i]);

        replyFiles[i].id = i;
        replyFiles[i].time = 0;
        replyFiles[i].size = fkfsFiles[i].size;
        replyFiles[i].pages = fkfsFiles[i].size / DefaultPageSize;
        replyFiles[i].version = fkfsFiles[i].version;
        replyFiles[i].name.funcs.encode = pb_encode_string;
        replyFiles[i].name.arg = fkfsFiles[i].name;
    }

    pb_array_t filesArray = {
        .length = numberOfFiles,
        .itemSize = sizeof(fk_app_File),
        .buffer = &replyFiles,
        .fields = fk_app_File_fields,
    };

    reply.m().type = fk_app_ReplyType_REPLY_FILES;
    reply.m().files.files.funcs.encode = pb_encode_array;
    reply.m().files.files.arg = (void *)&filesArray;

    if (!buffer.write(reply)) {
        log("Error writing reply");
    }
}

TaskEval FkfsReplies::downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    taskPool.clear();

    auto ptr = taskPool.malloc(sizeof(DownloadFileTask));
    fkfs_iterator_token_t *resumeToken = nullptr;
    auto rawToken = query.getDownloadToken();
    if (rawToken != nullptr && rawToken->length > 0) {
        fk_assert(rawToken->length == sizeof(fkfs_iterator_token_t));
        resumeToken = (fkfs_iterator_token_t *)rawToken->buffer;
    }

    downloadFileTask = new (ptr) DownloadFileTask(fs, query.m().downloadFile.id, resumeToken, reply, buffer);

    log("Created DownloadFileTask = %p (%lu free)", downloadFileTask, fk_free_memory());

    return TaskEval::pass(*downloadFileTask);
}

void FkfsReplies::eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_file_truncate(fs, query.m().eraseFile.id);

    queryFilesReply(query, reply, buffer);
}

void FkfsReplies::resetAll() {
    if (!fkfs_file_truncate_all(fs)) {
        log("Error: Unable to truncateAll");
    }

    if (!fkfs_initialize(fs, true)) {
        log("Error: Unable to resetAll");
    }

    NVIC_SystemReset();
}

void FkfsReplies::dataSetsReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_file_info_t fkfsFile;

    // My plan is to pusht his into FkfsData and possibly consolidate fkfs handling.
    fkfs_get_file(fs, dataFileId, &fkfsFile);

    fk_app_DataSet dataSets[] = {
        {
            .id = dataFileId,
            .sensor = 0,
            .time = 0,
            .size = fkfsFile.size,
            .pages = fkfsFile.size / DefaultPageSize,
            .hash = 0,
            .name = {
                .funcs = {
                    .encode = pb_encode_string,
                },
                .arg = (void *)fkfsFile.name,
            },
        },
    };

    pb_array_t data_sets_array = {
        .length = (size_t)(fkfsFile.size > 0 ? 1 : 0),
        .itemSize = sizeof(fk_app_DataSet),
        .buffer = &dataSets,
        .fields = fk_app_DataSet_fields,
    };

    reply.m().type = fk_app_ReplyType_REPLY_DATA_SETS;
    reply.m().dataSets.dataSets.funcs.encode = pb_encode_array;
    reply.m().dataSets.dataSets.arg = (void *)&data_sets_array;

    if (!buffer.write(reply)) {
        log("Error writing reply");
    }
}

TaskEval FkfsReplies::downloadDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    taskPool.clear();

    auto ptr = taskPool.malloc(sizeof(DownloadFileTask));
    fkfs_iterator_token_t *resumeToken = nullptr;
    auto rawToken = query.getDownloadToken();
    if (rawToken != nullptr && rawToken->length > 0) {
        fk_assert(rawToken->length == sizeof(fkfs_iterator_token_t));
        resumeToken = (fkfs_iterator_token_t *)rawToken->buffer;
    }

    downloadFileTask = new (ptr) DownloadFileTask(fs, dataFileId, resumeToken, reply, buffer);

    log("Created DownloadFileTask = %p (%lu free)", downloadFileTask, fk_free_memory());

    return TaskEval::pass(*downloadFileTask);
}

void FkfsReplies::eraseDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    // We only have one data set right now.
    fkfs_file_truncate(fs, dataFileId);

    dataSetsReply(query, reply, buffer);
}

void FkfsReplies::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln("Files", f, args);
    va_end(args);
}

DownloadFileTask::DownloadFileTask(fkfs_t *fs, uint8_t file, fkfs_iterator_token_t *resumeToken, AppReplyMessage &reply, MessageBuffer &buffer) :
    Task("DownloadFile"), reply(&reply), buffer(&buffer), iterator(*fs, file, resumeToken) {
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
