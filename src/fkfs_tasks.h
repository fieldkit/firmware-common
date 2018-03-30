#ifndef FK_FKFS_TASKS_H_INCLUDED
#define FK_FKFS_TASKS_H_INCLUDED

#include <fkfs.h>

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
    fkfs_file_iter_t iter = {
        .token = { 0 }
    };
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
    fkfs_iterator_token_t &resumeToken() {
        return iter.token;
    }
    size_t size() {
        return totalBytes;
    }

public:
    FkfsIterator(fkfs_t &fs);
    FkfsIterator(fkfs_t &fs, uint8_t file);
    FkfsIterator(fkfs_t &fs, uint8_t file, fkfs_iterator_token_t *resumeToken);

public:
    uint8_t fileNumber() {
        return file;
    }
    void truncateFile();
    void reopen(fkfs_iterator_token_t &token);
    void status();
    void beginning();
    void end();
    DataBlock move();
    bool isFinished() {
        return finished || totalBytes == iteratedBytes;
    }

private:
    void open(uint8_t newFile);
    void log(const char *f, ...) const;


};

}

#endif
