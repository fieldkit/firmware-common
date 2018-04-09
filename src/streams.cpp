#include "streams.h"
#include "varint_streams.h"

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

/**
 * Encodes an unsigned variable-length integer using the MSB algorithm.
 * This function assumes that the value is stored as little endian.
 * @param value The input value. Any standard integer type is allowed.
 * @param ptr A pointer to a piece of reserved memory. Must have a minimum size dependent on the input size (32 bit = 5 bytes, 64 bit = 10 bytes).
 * @return The number of bytes used in the output memory.
 */
template<typename int_t = uint64_t>
size_t encodeVarint(int_t value, uint8_t *ptr) {
    size_t outputSize = 0;
    // While more than 7 bits of data are left, occupy the last output byte
    // and set the next byte flag
    while (value > 127) {
        //|128: Set the next byte flag
        ptr[outputSize] = ((uint8_t)(value & 127)) | 128;
        //Remove the seven bits we just wrote
        value >>= 7;
        outputSize++;
    }
    ptr[outputSize++] = ((uint8_t)value) & 127;
    return outputSize;
}

/**
 * Decodes an unsigned variable-length integer using the MSB algorithm.
 * @param value A variable-length encoded integer of arbitrary size.
 * @param inputSize How many bytes are
 */
template<typename int_t = uint64_t>
size_t decodeVarint(uint8_t *ptr, int_t *ret, size_t size) {
    *ret = 0;
    for (size_t i = 0; i < size; i++) {
        *ret |= (ptr[i] & 127) << (7 * i);
        //If the next-byte flag is set
        if (!(ptr[i] & 128)) {
            return i + 1;
        }
    }
    return 0;
}

VarintEncodedStream::DataBlock VarintEncodedStream::read() {
    if (position == filled) {
        // printf("End of block\n");
        position = 0;
        filled = 0;
    }

    // Fill our buffer if we can.
    auto available = bp.size - filled;
    if (available > 0) {
        auto r = reader->read(bp.ptr + filled, available);
        if (r == Stream::EOS) {
            if (filled == 0) {
                return DataBlock{ nullptr, -1, -1, -1 };
            }
        }
        else {
            // printf("Read %d bytes (%d available)\n", r, available);
            filled += r;
        }
    }

    // We don't have a block, so we need a varint to get the size.
    if (totalSize == 0 || totalPosition == totalSize) {
        // printf("Reading size: %d %d\n", position, filled);
        auto bytes = decodeVarint<int32_t>(bp.ptr + position, &totalSize, filled);
        if (bytes == 0) {
            // If we can't get a size from a full buffer, something has gone horribly wrong.
            totalSize = 0;
            fk_assert(bp.size - filled > 0);
            return DataBlock{ };
        }

        // printf("New block: %d (%d bytes)\n", totalSize, bytes);

        position += bytes;
        totalPosition = 0;
    }

    auto leftInBlock = totalSize - totalPosition;
    auto bytes = filled - position;
    auto blockSize = leftInBlock > bytes ? bytes : leftInBlock;

    if (blockSize == 0) {
        return DataBlock{ };
    }

    auto block = DataBlock{
        bp.ptr + position,
        blockSize,
        totalPosition,
        totalSize,
    };

    // printf("Returning block: %d / %d (%d bytes)\n", blockSize, totalSize, totalPosition);

    position += blockSize;
    totalPosition += blockSize;

    return block;
}

}
