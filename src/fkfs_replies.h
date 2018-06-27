#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include "message_buffer.h"
#include "app_messages.h"
#include "download_file_task.h"
#include "task_container.h"

namespace fk {

class FileSystem;

class FkfsReplies {
private:
    FileSystem *fileSystem;
    TaskContainer<DownloadFileTask> downloadFileTask;

public:
    FkfsReplies(FileSystem &fileSystem);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    Task *downloadFileReply(CoreState &state, AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer, WifiConnection &connection);
    void eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void resetAll(CoreState &state);

private:
    void log(const char *f, ...) const;

};

}

#endif
