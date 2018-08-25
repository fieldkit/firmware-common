#include "app_messages.h"

namespace fk {

static inline bool pb_network_info_item_decode(pb_istream_t *stream, pb_array_t *array) {
    fk_app_NetworkInfo info;
    info.ssid.funcs.decode = pb_decode_string;
    info.ssid.arg = array->pool;
    info.password.funcs.decode = pb_decode_string;
    info.password.arg = array->pool;

    if (!pb_decode(stream, fk_app_NetworkInfo_fields, &info)) {
        return false;
    }

    // TODO: Wasteful.
    auto previous = (const void *)array->buffer;
    array->length++;
    array->buffer = array->pool->malloc(array->itemSize * array->length);
    void *ptr = ((uint8_t *)array->buffer) + ((array->length - 1) * array->itemSize);
    if (previous != nullptr) {
        memcpy(array->buffer, previous, ((array->length - 1) * array->itemSize));
    }
    memcpy(ptr, &info, array->itemSize);

    return true;
}

fk_app_WireMessageQuery *AppQueryMessage::forDecode() {
    networksArray = {
        .length = 0,
        .itemSize = sizeof(fk_app_NetworkInfo),
        .buffer = nullptr,
        .fields = fk_app_NetworkInfo_fields,
        .decode_item_fn = pb_network_info_item_decode,
        .pool = pool,
    };

    message.networkSettings.networks.funcs.decode = pb_decode_array;
    message.networkSettings.networks.arg = (void *)&networksArray;

    message.identity.device.funcs.decode = pb_decode_string;
    message.identity.device.arg = (void *)pool;
    message.identity.stream.funcs.decode = pb_decode_string;
    message.identity.stream.arg = (void *)pool;

    message.module.message.funcs.decode = pb_decode_data;
    message.module.message.arg = (void *)pool;

    return &message;
}

fk_app_WireMessageQuery *AppQueryMessage::forEncode() {
    return &message;
}

fk_app_WireMessageReply *AppReplyMessage::forDecode() {
    return &message;
}

fk_app_WireMessageReply *AppReplyMessage::forEncode() {
    if (message.fileData.data.arg != nullptr) {
        message.fileData.data.funcs.encode = pb_encode_data;
    }
    if (message.identity.device.arg != nullptr) {
        message.identity.device.funcs.encode = pb_encode_string;
    }
    if (message.identity.stream.arg != nullptr) {
        message.identity.stream.funcs.encode = pb_encode_string;
    }
    if (message.module.message.arg != nullptr) {
        message.module.message.funcs.encode = pb_encode_data;
    }
    return &message;
}

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
