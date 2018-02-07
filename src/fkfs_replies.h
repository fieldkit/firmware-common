#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include <fkfs.h>

#include "message_buffer.h"
#include "app_messages.h"
#include "fkfs_tasks.h"

namespace fk {

class DownloadFileTask : public Task {
private:
    AppReplyMessage *reply;
    MessageBuffer *buffer;
    FkfsIterator iterator;

public:
    DownloadFileTask(fkfs_t *fs, uint8_t file, fkfs_iterator_token_t *resumeToken, AppReplyMessage &reply, MessageBuffer &buffer);
    virtual ~DownloadFileTask() {
    }

public:
    TaskEval task() override;

};

class FkfsReplies {
private:
    fkfs_t *fs;
    uint8_t dataFileId{ 0 };
    Pool taskPool{ "Tasks", sizeof(DownloadFileTask) + 4 };
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
    void downloadDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseDataSetReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);

private:
    void sendPageOfFile(uint8_t id, size_t customPageSize, pb_data_t *incomingToken, AppReplyMessage &reply, MessageBuffer &buffer);
    void log(const char *f, ...) const;

};

}

#endif
