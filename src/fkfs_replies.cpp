#include <new>

#include "tuning.h"
#include "debug.h"
#include "utils.h"
#include "fkfs_replies.h"
#include "file_system.h"
#include "core_state.h"

namespace fk {

FkfsReplies::FkfsReplies(FileSystem &fileSystem) : fileSystem(&fileSystem) {
}

void FkfsReplies::queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    auto &files = fileSystem->files();
    auto numberOfFiles = files.numberOfFiles();
    auto numberOfVisibleFiles = 0;
    fk_app_File replyFiles[numberOfFiles];

    for (size_t i = 0; i < numberOfFiles; ++i) {
        auto &fd = files.file(i);

        if (!fileSystem->files().isInternal(fd)) {
            auto size = fileSystem->fs().file_size(fd);

            auto j = numberOfVisibleFiles;
            replyFiles[j].id = i;
            replyFiles[j].time = 0;
            replyFiles[j].version = 0;
            replyFiles[j].size = size;
            replyFiles[j].name.funcs.encode = pb_encode_string;
            replyFiles[j].name.arg = fd.name;
            numberOfVisibleFiles++;
        }
    }

    pb_array_t filesArray = {
        .length = (size_t)numberOfVisibleFiles,
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

Task *FkfsReplies::downloadFileReply(CoreState &state, AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer, WifiConnection &connection) {
    FileCopySettings settings{
        (FileNumber)query.m().downloadFile.id
    };
    return downloadFileTask.ready(*fileSystem, state, reply, buffer, connection, settings);
}

void FkfsReplies::eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    queryFilesReply(query, reply, buffer);
}

void FkfsReplies::resetAll(CoreState &state) {
    if (!fileSystem->format()) {
        return;
    }

    if (!fileSystem->setup()) {
        return;
    }

    state.started();

    log("Reset done.");
}

void FkfsReplies::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "Files", f, args);
    va_end(args);
}

}
