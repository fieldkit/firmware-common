#include <algorithm>

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
    return length_ + offset_;
}

size_t FileReader::tell() {
    return file_->tell();
}

int32_t FileReader::read() {
    fk_assert(false);
    return EOS;
}

bool FileReader::open(uint32_t offset, uint32_t length) {
    offset_ = offset;
    length_ = length;

    auto fileSize = file_->size();

    if (!file_->seek(offset_)) {
        logf(LogLevels::ERROR, "FileReader", "Failed to seek to %lu", offset_);
        done_ = true;
        opened_ = false;
        return false;
    }

    if (length_ == 0) {
        length_ = fileSize - offset_;
    }
    else {
        auto remaining = fileSize - offset_;
        if (length_ > remaining) {
            length_ = remaining;
        }
        else {
            length_ = length;
        }
    }

    done_ = false;
    opened_ = true;
    return true;
}

int32_t FileReader::read(uint8_t *ptr, size_t size) {
    auto position = 0;
    auto end = offset_ + length_;
    if (file_ != nullptr && !done_) {
        auto left = end - tell();
        auto remaining = std::min(size, (size_t)left);
        while (remaining > 0) {
            auto read = file_->read(ptr + position, remaining);
            if (read == 0) {
                done_ = true;
                break;
            }
            position += read;
            remaining -= read;
        }
        if (tell() == end) {
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
