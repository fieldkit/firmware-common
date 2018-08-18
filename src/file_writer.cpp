#include "file_writer.h"
#include "debug.h"

namespace fk {

FileWriter::FileWriter() {
}

FileWriter::FileWriter(phylum::BlockedFile &file) : file_(&file) {
}

int32_t FileWriter::write(uint8_t *ptr, size_t size) {
    return file_->write(ptr, size, true);
}

int32_t FileWriter::write(uint8_t byte) {
    fk_assert(false);
    return 0;
}

void FileWriter::close() {
    if (!closed_) {
        file_->close();
        closed_ = true;
    }
}

}
