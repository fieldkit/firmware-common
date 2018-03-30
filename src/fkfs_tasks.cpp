#include "fkfs_tasks.h"
#include "debug.h"

namespace fk {

FkfsIterator::FkfsIterator(fkfs_t &fs) : fs(&fs), file(0) {
}

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file, fkfs_iterator_token_t *resumeToken) : fs(&fs), file(file) {
    if (resumeToken != nullptr) {
        fkfs_file_iterator_resume(&fs, &iter, resumeToken);
    }
}

void FkfsIterator::open(uint8_t newFile) {
    file = newFile;
    beginning();
}

void FkfsIterator::beginning() {
    finished = false;
    startedAt = 0;
    iteratedBytes = 0;
    statusAt = 0;
    fkfs_file_iterator_create(fs, file, &iter);
    totalBytes = iter.token.size;
}

void FkfsIterator::end() {
    fkfs_file_iterator_move_end(fs, &iter);
}

void FkfsIterator::reopen(fkfs_iterator_token_t &position) {
    beginning();
    fkfs_file_iterator_reopen(fs, &iter, &position);
    if (!fkfs_file_iterator_valid(fs, &iter)) {
        fkfs_file_iterator_create(fs, file, &iter);
        totalBytes = iter.token.size;
    }
    else {
        totalBytes = iter.token.size - position.size;
    }

    debugfpln("FkfsIterator", "Reopen %d / %d size=(%lu->%lu = %d) (%lu, %d)->(%lu, %d)", file, iter.token.file,
              position.size, iter.token.size, totalBytes,
              iter.token.block, iter.token.offset,
              iter.token.lastBlock, iter.token.lastOffset);
}

void FkfsIterator::status() {
    auto elapsed = millis() - startedAt;
    auto complete = ((float)iteratedBytes / totalBytes) * 100.0f;
    auto speed = iteratedBytes > 0 ? iteratedBytes / ((float)elapsed / 1000.0f) : 0.0f;
    debugfpln("FkfsIterator", "%d/%d %lums %.2f %.2fbps", iteratedBytes, totalBytes, elapsed, complete, speed);
}

DataBlock FkfsIterator::move() {
    if (!finished) {
        if (startedAt == 0) {
            fkfs_file_info_t info = { 0 };
            if (!fkfs_get_file(fs, file, &info)) {
                debugfpln("FkfsIterator", "Error: Unable to get file information!");
                return DataBlock{ nullptr, 0 };
            }

            startedAt = millis();
            statusAt = startedAt;
            if (totalBytes == 0) {
                totalBytes = info.size;
            }
            iteratedBytes = 0;
            status();
        }

        if (millis() - statusAt > 1000) {
            status();
            statusAt = millis();
        }

        if (fkfs_file_iterate(fs, &config, &iter)) {
            iteratedBytes += iter.size;
            return DataBlock{ iter.data, iter.size };
        }
        else {
            if (fkfs_file_iterator_done(fs, &iter)) {
                finished = true;
                status();
            }
        }
    }
    return DataBlock{ nullptr, 0 };
}

void FkfsIterator::truncateFile() {
    fkfs_file_truncate(fs, file);
    beginning();
}

}
