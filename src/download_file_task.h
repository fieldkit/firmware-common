#ifndef FK_DOWNLOAD_FILE_TASK_H_INCLUDED
#define FK_DOWNLOAD_FILE_TASK_H_INCLUDED

#include "active_object.h"
#include "message_buffer.h"
#include "app_messages.h"
#include "wifi_client.h"
#include "files.h"

namespace fk {

class FileSystem;

class DownloadFileTask : public Task {
private:
    FileSystem *fileSystem;
    CoreState *state;
    AppReplyMessage *reply;
    MessageBuffer *buffer;
    WifiConnection *connection;
    FileCopySettings settings;
    uint32_t bytesCopied{ 0 };

public:
    DownloadFileTask(FileSystem &fileSystem, CoreState &state, AppReplyMessage &reply, MessageBuffer &buffer, WifiConnection &connection, FileCopySettings &settings);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
