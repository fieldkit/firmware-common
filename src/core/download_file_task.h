#ifndef FK_DOWNLOAD_FILE_TASK_H_INCLUDED
#define FK_DOWNLOAD_FILE_TASK_H_INCLUDED

#include "task.h"
#include "message_buffer.h"
#include "app_messages.h"
#include "wifi_client.h"
#include "files.h"
#include "core_state.h"

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
    bool began{ false };

public:
    DownloadFileTask(FileSystem &fileSystem, CoreState &state, AppReplyMessage &reply, MessageBuffer &buffer, WifiConnection &connection, FileCopySettings &settings);

public:
    void enqueued() override;
    TaskEval task() override;

private:
    bool writeHeader(uint32_t size);
    uint32_t calculateTotalSize(uint32_t metadataSize);

};

}

#endif
