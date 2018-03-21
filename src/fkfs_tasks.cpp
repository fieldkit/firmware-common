#include "fkfs_tasks.h"

namespace fk {

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file, fkfs_iterator_token_t *resumeToken) : fs(&fs), file(file) {
    if (resumeToken != nullptr) {
        fkfs_file_iterator_resume(&fs, file, &iter, resumeToken);
    }
}

void FkfsIterator::beginning() {
    finished = false;
    startedAt = 0;
    iteratedBytes = 0;
    statusAt = 0;
    iter = { 0 };
}

void FkfsIterator::end() {
    fkfs_file_iterator_move_end(fs, file, &iter);
}

void FkfsIterator::reopen(fkfs_iterator_token_t &position) {
    beginning();
    fkfs_file_iterator_reopen(fs, file, &iter, &position);
    totalBytes = iter.token.size - position.size;
    debugfpln("FkfsIterator", "Reopen size=(%lu->%lu) (%lu, %d)->(%lu, %d)", position.size, iter.token.size, iter.token.block, iter.token.offset, iter.token.lastBlock, iter.token.lastOffset);
}

void FkfsIterator::resume(fkfs_iterator_token_t &position) {
    beginning();
    fkfs_file_iterator_resume(fs, file, &iter, &position);
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

        if (fkfs_file_iterate(fs, file, &config, &iter)) {
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
