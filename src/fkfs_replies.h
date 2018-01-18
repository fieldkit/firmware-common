#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include <fkfs.h>

#include "message_buffer.h"
#include "app_messages.h"

namespace fk {

class FkfsReplies {
private:
    fkfs_t *fs;
    uint8_t dataFileId{0};

public:
    FkfsReplies(fkfs_t &fs, uint8_t dataFileId);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
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
