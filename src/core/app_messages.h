#ifndef FK_APP_MESSAGES_H_INCLUDED
#define FK_APP_MESSAGES_H_INCLUDED

#include <fk-app-protocol.h>

#include "debug.h"
#include "pool.h"
#include "protobuf.h"

namespace fk {

class AppQueryMessage {
public:
    static constexpr const pb_msgdesc_t *fields{ fk_app_WireMessageQuery_fields };

private:
    fk_app_WireMessageQuery message = fk_app_WireMessageQuery_init_default;
    pb_array_t networksArray;
    Pool *pool;

public:
    AppQueryMessage(Pool *pool) : pool(pool) {
    }

    void clear() {
        message = fk_app_WireMessageQuery_init_default;
    }

    fk_app_WireMessageQuery *forDecode();

    fk_app_WireMessageQuery *forEncode();

    fk_app_WireMessageQuery &m() {
        return message;
    }

};

class AppReplyMessage {
public:
    static constexpr const pb_msgdesc_t *fields{ fk_app_WireMessageReply_fields };

private:
    fk_app_WireMessageReply message = fk_app_WireMessageReply_init_default;
    Pool *pool;

public:
    AppReplyMessage(Pool *pool) : pool(pool) {
    }

    void clear() {
        message = fk_app_WireMessageReply_init_default;
    }

    fk_app_WireMessageReply *forDecode();

    fk_app_WireMessageReply *forEncode();

    fk_app_WireMessageReply &m() {
        return message;
    }

public:
    void busy(const char *text);
    void error(const char *text);

};

}

#endif
