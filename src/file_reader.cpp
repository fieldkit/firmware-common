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

size_t FileReader::size() {
    return size_;
}

size_t FileReader::tell() {
    return file_->tell();
}

int32_t FileReader::read() {
    fk_assert(false);
    return EOS;
}

void FileReader::open() {
    file_->seek(0);
    size_ = file_->size();
    done_ = false;
    opened_ = true;
}

int32_t FileReader::read(uint8_t *ptr, size_t size) {
    auto position = 0;
    if (file_ != nullptr && !done_) {
        auto left = size_ - tell();
        auto remaining = std::min(size, left);
        while (remaining > 0) {
            auto read = file_->read(ptr + position, remaining);
            if (read == 0) {
                done_ = true;
                break;
            }
            position += read;
            remaining -= read;
        }
        if (tell() == size_) {
            done_ = true;
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
