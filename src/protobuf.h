#ifndef FK_PROTOBUF_H_INCLUDED
#define FK_PROTOBUF_H_INCLUDED

#include <pb_encode.h>
#include <pb_decode.h>

namespace fk {

bool pb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool pb_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);

typedef struct pb_array_t {
    size_t length;
    size_t itemSize;
    const void *buffer;
    const pb_field_t *fields;
} pb_array_t;

bool pb_encode_array(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

}

#endif
