#include "file_copy_operation.h"
#include "platform.h"

namespace fk {

FileCopyOperation::FileCopyOperation() {
}

bool FileCopyOperation::isFinished() const {
    return !busy_;
}

size_t FileCopyOperation::tell() {
    return reader_.tell();
}

size_t FileCopyOperation::size() {
    return reader_.size();
}

size_t FileCopyOperation::remaining() {
    return size() - tell();
}

uint32_t FileCopyOperation::version() const {
    return reader_.version();
}

bool FileCopyOperation::prepare(const FileReader &reader, const FileCopySettings &settings) {
    reader_ = reader;

    streamCopier_.restart();

    if (!reader_.open(settings.offset, settings.length)) {
        return false;
    }

    started_ = 0;
    copied_ = 0;
    busy_ = true;
    total_ = reader_.size() - reader_.tell();

    return true;
}

bool FileCopyOperation::copy(lws::Writer &writer, FileCopyCallbacks *callbacks) {
    if (started_ == 0) {
        started_ = fk_uptime();
        status_ = fk_uptime();
    }

    auto started = fk_uptime();
    while (fk_uptime() - started < FileCopyMaximumElapsed) {
        auto bytes = streamCopier_.copy(reader_, writer);
        if (bytes == 0) {
            break;
        }

        if (bytes > 0) {
            copied_ += bytes;
        }

        if (bytes == lws::Stream::EOS) {
            status();
            busy_ = false;
            break;
        }

        if (fk_uptime() - status_ > FileCopyStatusInterval) {
            status();
            status_ = fk_uptime();
        }

        if (callbacks != nullptr) {
            callbacks->fileCopyTick();
        }
    }

    return true;
}

void FileCopyOperation::status() {
    auto elapsed = fk_uptime() - started_;
    auto complete = copied_ > 0 ? ((float)copied_ / total_) * 100.0f : 0.0f;
    auto speed = copied_ > 0 ? copied_ / ((float)elapsed / 1000.0f) : 0.0f;
    logtracef("Copy", "%lu/%lu %lums %.2f %.2fbps", copied_, total_, elapsed, complete, speed);
}

}
