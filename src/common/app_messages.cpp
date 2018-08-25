#include "app_messages.h"

namespace fk {

void AppReplyMessage::busy(const char *text) {
    fk_app_Error errors[] = {
        {
            .message = {
                .funcs = {
                    .encode = pb_encode_string,
                },
                .arg = (void *)text,
            },
        }
    };

    pb_array_t errors_array = {
        .length = sizeof(errors) / sizeof(fk_app_Error),
        .itemSize = sizeof(fk_app_Error),
        .buffer = pool->copy(errors, sizeof(errors)),
        .fields = fk_app_Error_fields,
    };

    message.type = fk_app_ReplyType_REPLY_BUSY;
    message.errors.funcs.encode = pb_encode_array;
    message.errors.arg = (void *)pool->copy(&errors_array, sizeof(errors_array));
}

void AppReplyMessage::error(const char *text) {
    fk_app_Error errors[] = {
        {
            .message = {
                .funcs = {
                    .encode = pb_encode_string,
                },
                .arg = (void *)text,
            },
        }
    };

    pb_array_t errors_array = {
        .length = sizeof(errors) / sizeof(fk_app_Error),
        .itemSize = sizeof(fk_app_Error),
        .buffer = pool->copy(errors, sizeof(errors)),
        .fields = fk_app_Error_fields,
    };

    message.type = fk_app_ReplyType_REPLY_ERROR;
    message.errors.funcs.encode = pb_encode_array;
    message.errors.arg = (void *)pool->copy(&errors_array, sizeof(errors_array));
}

}
