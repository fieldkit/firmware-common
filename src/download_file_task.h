#ifndef FK_DOWNLOAD_FILE_TASK_H_INCLUDED
#define FK_DOWNLOAD_FILE_TASK_H_INCLUDED

#include "active_object.h"
#include "message_buffer.h"
#include "app_messages.h"
#include "wifi_client.h"

namespace fk {

class FileSystem;

class DownloadFileTask : public Task {
private:
    FileSystem *fileSystem;
    CoreState *state;
    AppReplyMessage *reply;
    MessageBuffer *buffer;
    WifiConnection *connection;
    uint32_t bytesCopied{ 0 };

public:
    DownloadFileTask(FileSystem &fileSystem, CoreState &state, AppReplyMessage &reply, MessageBuffer &buffer, WifiConnection &connection);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
