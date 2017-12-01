#include "protobuf.h"
#include "debug.h"
#include "pool.h"

namespace fk {

bool pb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    const char *str = (const char *)*arg;
    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

bool pb_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    Pool *pool = (Pool *)(*arg);
    auto len = stream->bytes_left;

    auto *ptr = (uint8_t *)pool->malloc(len + 1);
    if (!pb_read(stream, ptr, len)) {
        return false;
    }

    ptr[len] = 0;

    (*arg) = (void *)ptr;

    return true;
}

}
