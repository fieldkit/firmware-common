#include "file_reader.h"
#include "debug.h"

namespace fk {

FileReader::FileReader() {
}

FileReader::FileReader(phylum::SimpleFile *file) : file_(file) {
}

bool FileReader::isOpen() {
    return file_ != nullptr && opened_;
}

bool FileReader::isFinished() {
    return file_ != nullptr && done_;
}

void FileReader::open() {
    file_->seek(0);
    done_ = false;
    opened_ = true;
}

void FileReader::end() {
    file_->seek(UINT64_MAX);
}

size_t FileReader::size() {
    return file_->size();
}

size_t FileReader::tell() {
    return file_->tell();
}

void FileReader::truncate() {
}

int32_t FileReader::read() {
    fk_assert(false);
    return EOS;
}

int32_t FileReader::read(uint8_t *ptr, size_t size) {
    auto position = 0;
    if (file_ != nullptr && !done_) {
        auto remaining = size;
        while (remaining > 0) {
            auto read = file_->read(ptr + position, remaining);
            if (read == 0) {
                done_ = true;
                break;
            }
            position += read;
            remaining -= read;
        }
    }

    if (position == 0) {
        return EOS;
    }

    return position;
}

void FileReader::close() {
    opened_ = false;
}

}
