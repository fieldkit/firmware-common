#include "streams.h"

namespace fk {

int32_t StreamCopier::copy(Reader &reader, Writer &writer) {
    auto available = buffer.size - position;
    auto eos = false;
    auto copied = 0;

    if (available > 0) {
        auto read = reader.read(buffer.ptr + position, available);
        if (read == Stream::EOS) {
            eos = true;
        }
        if (read > 0) {
            position += read;
        }
    }

    if (position > 0) {
        auto wrote = writer.write(buffer.ptr, position);
        if (wrote > 0) {
            copied = wrote;
            if (wrote != (int32_t)position) {
                memmove(buffer.ptr, buffer.ptr + wrote, wrote);
                position -= wrote;
            }
            else {
                position = 0;
            }
        }
    }

    if (position == 0 && eos) {
        return Stream::EOS;
    }

    return copied;
}

}
