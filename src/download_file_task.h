#ifndef FK_DOWNLOAD_FILE_TASK_H_INCLUDED
#define FK_DOWNLOAD_FILE_TASK_H_INCLUDED

#include <fkfs.h>

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

public:
    TaskEval task() override;

};

}

#endif
