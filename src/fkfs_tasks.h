#ifndef FK_FKFS_TASKS_H_INCLUDED
#define FK_FKFS_TASKS_H_INCLUDED

#include <fkfs.h>
#include "active_object.h"

namespace fk {

struct DataBlock {
    void *ptr;
    size_t size;

    operator bool() const {
        return ptr != nullptr;
    }
};

class FkfsIterator {
private:
    fkfs_t *fs{ nullptr };
    uint8_t file{ 0 };
    fkfs_iterator_token_t token = { 0 };
    fkfs_file_iter_t iter = { 0 };
    fkfs_iterator_config_t config = {
        .maxBlocks = 0,
        .maxTime = 250
    };
    bool finished{ false };
    uint32_t startedAt{ 0 };
    uint32_t statusAt{ 0 };
    size_t totalBytes{ 0 };
    size_t iteratedBytes{ 0 };

public:
    fkfs_iterator_token_t *resumeToken() {
        return &token;
    }

public:
    FkfsIterator(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
    }

    FkfsIterator(fkfs_t &fs, uint8_t file, fkfs_iterator_token_t *resumeToken) : fs(&fs), file(file) {
        if (resumeToken != nullptr) {
            memcpy(&token, resumeToken, sizeof(fkfs_iterator_token_t));
        }
    }

public:
    void resume(fkfs_iterator_token_t &resumeToken) {
        token = resumeToken;
    }

    void status() {
        debugfpln("FkfsIterator", "%d/%d bytes, %lums, %.2f", iteratedBytes, totalBytes,
                  millis() - startedAt, ((float)iteratedBytes / totalBytes) * 100.0f);
    }

    DataBlock move() {
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

    bool handle(void *data, size_t size) {
        return true;
    }

    bool isFinished() {
        return finished;
    }

};

class DataIteratorTask : public Task {
private:
    fkfs_t *fs;
    FkfsIterator iterator;

public:
    DataIteratorTask(fkfs_t &fs);

public:
    TaskEval task();

};

}

#endif
