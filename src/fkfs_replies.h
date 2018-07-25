#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include "message_buffer.h"
#include "app_messages.h"

namespace fk {

class FileSystem;

class FkfsReplies {
private:
    FileSystem *fileSystem;

public:
    FkfsReplies(FileSystem &fileSystem);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseAll(CoreState &state);

private:
    void log(const char *f, ...) const;

};

}

#endif
