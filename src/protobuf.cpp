#include <fk-app-protocol.h>

#include "protobuf.h"
#include "debug.h"
#include "pool.h"

namespace fk {

constexpr char LOG[] = "PROTOBUF";

bool pb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    auto str = (const char *)*arg;
    if (str == nullptr) {
        return pb_encode_string(stream, (uint8_t *)nullptr, 0);
    }

#ifdef FK_PROTOBUF_VERBOSE
    debugfpln(LOG, "Encode: 0x%x '%s'", str, str != nullptr ? str : "");
#endif

    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

bool pb_decode_string(pb_istream_t *stream, const pb_field_t *, void **arg) {
    auto pool = (Pool *)(*arg);
    auto len = stream->bytes_left;

    if (len == 0) {
#ifdef FK_PROTOBUF_VERBOSE
        debugfpln(LOG, "Decode: EMPTY");
#endif
        (*arg) = (void *)"";
        return true;
    }

    auto *ptr = (uint8_t *)pool->malloc(len + 1);
    if (!pb_read(stream, ptr, len)) {
        return false;
    }

    ptr[len] = 0;

#ifdef FK_PROTOBUF_VERBOSE
    debugfpln(LOG, "Decode: '%s' (%d)", ptr, len);
#endif

    (*arg) = (void *)ptr;

    return true;
}

bool pb_encode_array(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    auto array = (pb_array_t *)*arg;

    auto ptr = (uint8_t *)array->buffer;
    for (size_t i = 0; i < array->length; ++i) {
        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }

        if (!pb_encode_submessage(stream, array->fields, ptr)) {
            return false;
        }

        ptr += array->itemSize;
    }

    return true;
}

bool pb_encode_uint32_array(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    auto array = (pb_array_t *)*arg;

    auto ptr = (uint32_t *)array->buffer;
    for (size_t i = 0; i < array->length; ++i) {
        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }

        if (!pb_encode_varint(stream, *ptr)) {
            return false;
        }

        ptr++;
    }

    return true;
}

bool pb_decode_array(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    auto array = (pb_array_t *)*arg;

    // HACK: This is a hack. I'm wondering if there's a better way around this.
    // Maybe a template pointer of some kind?
    if (array->fields == fk_app_NetworkInfo_fields) {
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
    } else {
        debugfpln(LOG, "Error in pb_decode_array. Unknown type.");
    }

    return true;
}

bool pb_encode_data(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    auto data = (pb_data_t *)*arg;

    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    if (!pb_encode_varint(stream, data->length)) {
        return false;
    }

    auto ptr = (uint8_t *)data->buffer;
    if (ptr != nullptr) {
        if (!pb_write(stream, ptr, data->length)) {
            return false;
        }
    }

#ifdef FK_PROTOBUF_VERBOSE
    debugfpln(LOG, "Encode: (%d)", data->length);
#endif

    return true;
}

bool pb_decode_data(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    auto pool = (Pool *)(*arg);
    auto length = stream->bytes_left;

#ifdef FK_PROTOBUF_VERBOSE
    debugfpln(LOG, "Decode: (%d)", length);
#endif

    auto data = (pb_data_t *)pool->malloc(sizeof(pb_data_t) + length);
    data->buffer = ((uint8_t *)data) + sizeof(pb_data_t);
    data->length = length;

    if (!pb_read(stream, (pb_byte_t *)data->buffer, length)) {
        return false;
    }

#ifdef FK_PROTOBUF_VERBOSE
    debugfpln(LOG, "Decode(done): (%d)", length);
#endif

    (*arg) = (void *)data;

    return true;
}

}
