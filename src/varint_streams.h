#pragma once

#include "streams.h"

namespace fk {

class VarintEncodedStream {
public:
    struct DataBlock {
        uint8_t *ptr;
        int32_t blockSize;
        int32_t position;
        int32_t totalSize;

        operator bool() {
            return totalSize > 0;
        }

        bool beginning() {
            return position == 0;
        }

        bool eos() {
            return totalSize == Stream::EOS;
        }
    };

private:
    Reader *reader;
    BufferPtr bp;
    int32_t filled{ 0 };
    int32_t position{ 0 };
    int32_t totalSize{ 0 };
    int32_t totalPosition{ 0 };

public:
    VarintEncodedStream(Reader &reader, BufferPtr bp) : reader(&reader), bp(bp) {
    }

public:
    DataBlock read();

};

}
