#ifndef FK_FKFS_REPLIES_H_INCLUDED
#define FK_FKFS_REPLIES_H_INCLUDED

#include <fkfs.h>

#include "app_messages.h"

namespace fk {

class FkfsReplies {
private:
    fkfs_t *fs;

public:
    FkfsReplies(fkfs_t &fs);

public:
    void queryFilesReply(AppQueryMessage &query, AppReplyMessage &reply);
    void downloadFileReply(AppQueryMessage &query, AppReplyMessage &reply);

};

}

#endif
