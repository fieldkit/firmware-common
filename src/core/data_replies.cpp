#include <new>

#include "tuning.h"
#include "debug.h"
#include "data_replies.h"
#include "file_system.h"
#include "core_state.h"
#include "file_cursors.h"

namespace fk {

constexpr const char Log[] = "Files";

using Logger = SimpleLog<Log>;

DataReplies::DataReplies(FileSystem &fileSystem) : fileSystem(&fileSystem) {
}

void DataReplies::queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    auto &files = fileSystem->files();
    auto numberOfFiles = files.numberOfFiles();
    auto numberOfVisibleFiles = 0;
    fk_app_File replyFiles[numberOfFiles];

    for (size_t i = 0; i < numberOfFiles; ++i) {
        auto &fd = files.file(i);

        if (!fileSystem->files().isInternal(fd)) {
            auto stat = fileSystem->fs().stat(fd);
            auto j = numberOfVisibleFiles;
            replyFiles[j].id = i;
            replyFiles[j].time = 0;
            replyFiles[j].version = stat.version;
            replyFiles[j].size = stat.size;
            replyFiles[j].maximum = fd.maximum_size;
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
        Logger::error("Error writing reply");
    }
}

void DataReplies::eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer) {
    auto number = (FileNumber)query.m().eraseFile.id;

    if (!fileSystem->erase(number)) {
        Logger::error("Failed to erase file: %d", number);
    }

    FileCursorManager fcm(*fileSystem);
    if (!fcm.save(number, 0)) {
        Logger::error("Failed to save cursor: %d", sizeof(FileCursors));
    }

    queryFilesReply(query, reply, buffer);
}

void DataReplies::eraseAll(CoreState &state) {
    /*
    if (!fileSystem->format()) {
        return;
    }

    if (!fileSystem->setup()) {
        return;
    }
    */

    if (!fileSystem->eraseAll()) {
        return;
    }

    state.started();

    Logger::info("Reset done.");
}

}
