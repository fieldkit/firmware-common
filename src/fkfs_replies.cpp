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
    size_t length = 0;

    auto rawPreviousToken = (pb_data_t *)query.m().downloadFile.token.arg;
    if (query.hasToken() && rawPreviousToken != nullptr && rawPreviousToken->length > 0) {
        fk_assert(rawPreviousToken->length == sizeof(fkfs_iterator_token_t));
        memcpy((void *)&token, (void *)rawPreviousToken->buffer, sizeof(fkfs_iterator_token_t));
        debugfpln("Files", "Using previous token (%d, %d -> %d)", token.block, token.offset, token.lastBlock);
    } else {
        debugfpln("Files", "Starting download");
    }

    while (fkfs_file_iterate(fs, query.m().downloadFile.id, &iter, &token)) {
        if (length + iter.size >= sizeof(data)) {
            break;
        }

        memcpy(data + length, iter.data, iter.size);

        length += iter.size;
    }

    pb_data_t dataData = {
        .length = length,
        .buffer = data,
    };

    pb_data_t tokenData = {
        .length = sizeof(fkfs_iterator_token_t),
        .buffer = &token,
    };

    debugfpln("Files", "Done (%d bytes), sending token (%d, %d -> %d)", length, token.block, token.offset, token.lastBlock);

    reply.m().type = fk_app_ReplyType_REPLY_DOWNLOAD_FILE;
    reply.m().fileData.data.funcs.encode = pb_encode_data;
    reply.m().fileData.data.arg = (void *)&dataData;
    reply.m().fileData.token.funcs.encode = pb_encode_data;
    reply.m().fileData.token.arg = (void *)&tokenData;

    if (!buffer.write(reply)) {
        debugfpln("Error", "Error writing reply");
    }
}

}
