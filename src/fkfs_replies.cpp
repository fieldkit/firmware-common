#include "fkfs_replies.h"
#include "debug.h"

namespace fk {

constexpr uint32_t DefaultPageSize = (size_t)(8 * 4096);

FkfsReplies::FkfsReplies(fkfs_t &fs) : fs(&fs) {
}

void FkfsReplies::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln("Files", f, args);
    va_end(args);
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

void FkfsReplies::sendPageOfFile(uint8_t id, size_t customPageSize, pb_data_t *incomingToken, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_iterator_token_t token = { 0 };
    fkfs_file_iter_t iter = { 0 };
    uint8_t data[1024];
    size_t total = 0;
    auto started = millis();
    auto buffersSent = 0;
    auto pageSize = DefaultPageSize;
    if (customPageSize > 0) {
        pageSize = customPageSize;
    }

    if (incomingToken != nullptr && incomingToken->length > 0) {
        fk_assert(incomingToken->length == sizeof(fkfs_iterator_token_t));
        memcpy((void *)&token, (void *)incomingToken->buffer, sizeof(fkfs_iterator_token_t));
        log("Using previous token (pageSize = %lu) (%lu, %d -> %lu)", pageSize, token.block, token.offset, token.lastBlock);
    } else {
        log("Starting download (pageSize = %lu)", pageSize);
    }

    pb_data_t dataData = {
        .length = 0,
        .buffer = data,
    };

    pb_data_t tokenData = {
        .length = sizeof(fkfs_iterator_token_t),
        .buffer = &token,
    };

    reply.m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
    reply.m().fileData.data.funcs.encode = pb_encode_data;
    reply.m().fileData.data.arg = (void *)&dataData;
    reply.m().fileData.token.funcs.encode = pb_encode_data;
    reply.m().fileData.token.arg = (void *)&tokenData;

    while (fkfs_file_iterate(fs, id, &iter, &token)) {
        if (dataData.length + iter.size >= sizeof(data)) {
            if (!buffer.write(reply)) {
                log("Error writing reply");
            }

            buffer.write();
            buffersSent++;

            dataData.length = 0;
        }

        memcpy(data + dataData.length, iter.data, iter.size);

        dataData.length += iter.size;
        total += iter.size;

        if (total >= pageSize) {
            break;
        }
    }

    if (buffer.position() > 0 || buffersSent == 0) {
        if (!buffer.write(reply)) {
            log("Error writing reply");
        }
        buffer.write();
    }

    log("Done (%d bytes), sending token (%lu, %d -> %lu) (took %lu)", total, token.block, token.offset, token.lastBlock, millis() - started);
}

void FkfsReplies::downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    sendPageOfFile(query.m().downloadFile.id, query.m().downloadFile.pageSize, query.getDownloadToken(), reply, buffer);
}

void FkfsReplies::eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_file_truncate(fs, query.m().eraseFile.id);

    return queryFilesReply(query, reply, buffer);
}

void FkfsReplies::resetAll() {
    fkfs_file_truncate_all(fs);
}

void FkfsReplies::dataSetsReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_file_info_t fkfsFile;

    // My plan is to pusht his into FkfsData and possibly consolidate fkfs handling.
    fkfs_get_file(fs, 1, &fkfsFile);

    fk_app_DataSet dataSets[] = {
        {
            .id = 1,
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

void FkfsReplies::downloadDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    sendPageOfFile(1, query.m().downloadDataSet.pageSize, query.getDownloadToken(), reply, buffer);
}

void FkfsReplies::eraseDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    // We only have one data set right now.
    fkfs_file_truncate(fs, 1);

    dataSetsReply(query, reply, buffer);
}

}
