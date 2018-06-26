#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include "message_buffer.h"
#include "app_messages.h"
#include "download_file_task.h"
#include "task_container.h"

namespace fk {

class FkfsReplies {
private:
    TaskContainer<DownloadFileTask> downloadFileTask;

public:
    FkfsReplies();

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    Task *downloadFileReply(CoreState &state, AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void resetAll();

public:
    void dataSetsReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    Task *downloadDataSetReply(CoreState &state, AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);

private:
    void log(const char *f, ...) const;

};

}

#endif
