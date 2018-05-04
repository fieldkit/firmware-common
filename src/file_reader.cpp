#include "file_reader.h"

namespace fk {

FileReader::FileReader(FileSystem &fileSystem, uint8_t file) : iterator(fileSystem.fkfs(), file) {
}

void FileReader::open() {
    iterator.beginning();
}

void FileReader::end() {
    iterator.end();
}

void FileReader::truncate() {
    iterator.truncate();
}

void FileReader::open(fkfs_iterator_token_t &token) {
    iterator.reopen(token);
}

int32_t FileReader::read() {
    fk_assert(false);
    return EOS;
}

int32_t FileReader::read(uint8_t *ptr, size_t size) {
    if (iterator.isFinished()) {
        return EOS;
    }

    auto position = 0;
    auto remaining = size;
    while (remaining > 0 && !iterator.isFinished()) {
        auto data = iterator.read(remaining);
        if (data) {
            fk_assert(data.size <= size);
            memcpy(ptr + position, data.ptr, data.size);
            remaining -= data.size;
            position += data.size;
            iterator.moveNext(data);
        }
    }

    return position;
}

void FileReader::close() {
}

}
