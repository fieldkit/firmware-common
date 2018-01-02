#include "fkfs_replies.h"
#include "debug.h"

namespace fk {

FkfsReplies::FkfsReplies(fkfs_t &fs) : fs(&fs) {
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
        replyFiles[i].pages = fkfsFiles[i].size / 1024;
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
        debugfpln("Error", "Error writing reply");
    }
}

void FkfsReplies::downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_iterator_token_t token = { 0 };
    fkfs_file_iter_t iter = { 0 };
    uint8_t data[1024];
    size_t total = 0;
    auto started = millis();
    auto buffersSent = 0;

    auto rawPreviousToken = (pb_data_t *)query.m().downloadFile.token.arg;
    if (query.hasToken() && rawPreviousToken != nullptr && rawPreviousToken->length > 0) {
        fk_assert(rawPreviousToken->length == sizeof(fkfs_iterator_token_t));
        memcpy((void *)&token, (void *)rawPreviousToken->buffer, sizeof(fkfs_iterator_token_t));
        debugfpln("Files", "Using previous token (%lu, %d -> %lu)", token.block, token.offset, token.lastBlock);
    } else {
        debugfpln("Files", "Starting download");
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

    while (fkfs_file_iterate(fs, query.m().downloadFile.id, &iter, &token)) {
        if (dataData.length + iter.size >= sizeof(data)) {
            if (!buffer.write(reply)) {
                debugfpln("Error", "Error writing reply");
            }

            buffer.write();
            buffersSent++;

            dataData.length = 0;
        }

        memcpy(data + dataData.length, iter.data, iter.size);

        dataData.length += iter.size;
        total += iter.size;

        if (total >= 4096 * 8) {
            break;
        }
    }

    if (buffer.position() > 0 || buffersSent == 0) {
        if (!buffer.write(reply)) {
            debugfpln("Error", "Error writing reply");
        }
        buffer.write();
    }

    debugfpln("Files", "Done (%d bytes), sending token (%lu, %d -> %lu) (took %lu)", total, token.block, token.offset, token.lastBlock, millis() - started);
}

void FkfsReplies::eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    fkfs_file_truncate(fs, query.m().eraseFile.id);

    return queryFilesReply(query, reply, buffer);
}

void FkfsReplies::resetAll() {
    fkfs_file_truncate_all(fs);
}

}
