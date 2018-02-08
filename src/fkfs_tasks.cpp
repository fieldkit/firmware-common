#include "fkfs_tasks.h"

namespace fk {

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

FkfsIterator::FkfsIterator(fkfs_t &fs, uint8_t file, fkfs_iterator_token_t *resumeToken) : fs(&fs), file(file) {
    if (resumeToken != nullptr) {
        memcpy(&token, resumeToken, sizeof(fkfs_iterator_token_t));
    }
}

void FkfsIterator::resume(fkfs_iterator_token_t &resumeToken) {
    token = resumeToken;
}

void FkfsIterator::status() {
    debugfpln("FkfsIterator", "%d/%d bytes, %lums, %.2f", iteratedBytes, totalBytes, millis() - startedAt, ((float)iteratedBytes / totalBytes) * 100.0f);
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
            totalBytes = info.size;
            iteratedBytes = 0;
            status();
        }

        if (millis() - statusAt > 1000) {
            status();
            statusAt = millis();
        }

        if (fkfs_file_iterate(fs, file, &config, &iter, &token)) {
            iteratedBytes += iter.size;
            return DataBlock{ iter.data, iter.size };
        }
        else {
            finished = true;
            status();
        }
    }
    return DataBlock{ nullptr, 0 };
}

}
