#ifndef FK_DOWNLOAD_FILE_TASK_H_INCLUDED
#define FK_DOWNLOAD_FILE_TASK_H_INCLUDED

#include "active_object.h"
#include "message_buffer.h"
#include "app_messages.h"

namespace fk {

class DownloadFileTask : public Task {
private:
    CoreState *state;
    AppReplyMessage *reply;
    MessageBuffer *buffer;
    uint32_t bytesCopied{ 0 };

public:
    DownloadFileTask(CoreState &state, AppReplyMessage &reply, MessageBuffer &buffer);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
