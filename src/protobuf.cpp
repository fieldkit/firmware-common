#include "protobuf.h"
#include "debug.h"
#include "pool.h"

namespace fk {

constexpr char LOG[] = "PROTOBUF";

bool pb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
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

bool pb_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
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

}
