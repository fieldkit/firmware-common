#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include <fkfs.h>

#include "message_buffer.h"
#include "app_messages.h"
#include "download_file_task.h"

namespace fk {

class FkfsReplies {
private:
    fkfs_t *fs;
    uint8_t dataFileId{ 0 };
    Pool taskPool{ "Tasks", sizeof(DownloadFileTask) };
    DownloadFileTask *downloadFileTask;

public:
    FkfsReplies(fkfs_t &fs, uint8_t dataFileId);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    TaskEval downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void resetAll();

public:
    void dataSetsReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    TaskEval downloadDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);

private:
    void log(const char *f, ...) const;

};

}

#endif
