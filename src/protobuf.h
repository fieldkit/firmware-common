#ifndef FK_PROTOBUF_H_INCLUDED
#define FK_PROTOBUF_H_INCLUDED

#include <pb_encode.h>
#include <pb_decode.h>

namespace fk {

bool pb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool pb_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);

}

#endif
