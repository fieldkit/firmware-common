#ifndef FK_DATA_REPLIES_H_INCLUDED
#define FK_DATA_REPLIES_H_INCLUDED

#include "message_buffer.h"
#include "app_messages.h"
#include "core_state.h"

namespace fk {

class FileSystem;

class DataReplies {
private:
    FileSystem *fileSystem;

public:
    DataReplies(FileSystem &fileSystem);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseFileReply(AppQueryMessage &query, AppReplyMessage &reply, MessageBuffer &buffer);
    void eraseAll(CoreState &state);

};

}

#endif
