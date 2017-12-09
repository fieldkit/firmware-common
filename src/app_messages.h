#ifndef FK_APP_MESSAGES_H_INCLUDED
#define FK_APP_MESSAGES_H_INCLUDED

#include <fk-app-protocol.h>

#include "pool.h"
#include "protobuf.h"

namespace fk {

class AppQueryMessage {
private:
    fk_app_WireMessageQuery message = fk_app_WireMessageQuery_init_default;
    Pool *pool;

public:
    AppQueryMessage(Pool *pool) : pool(pool) {
    }

    bool hasToken() {
        return message.downloadFile.token.arg != pool;
    }

    fk_app_WireMessageQuery *forDecode() {
        message.downloadFile.token.funcs.decode = pb_decode_data;
        message.downloadFile.token.arg = (void *)pool;
        return &message;
    }

    fk_app_WireMessageQuery *forEncode() {
        return &message;
    }

    fk_app_WireMessageQuery &m() {
        return message;
    }

};

class AppReplyMessage {
private:
    fk_app_WireMessageReply message = fk_app_WireMessageReply_init_default;
    Pool *pool;

public:
    AppReplyMessage(Pool *pool) : pool(pool) {
    }

    fk_app_WireMessageReply *forDecode() {
        return &message;
    }

    fk_app_WireMessageReply *forEncode() {
        if (message.type == fk_app_ReplyType_REPLY_DOWNLOAD_FILE) {
            message.fileData.token.funcs.encode = pb_encode_data;
            message.fileData.data.funcs.encode = pb_encode_data;
        }
        return &message;
    }

    fk_app_WireMessageReply &m() {
        return message;
    }

public:
    void error(const char *error);

};

}

#endif
