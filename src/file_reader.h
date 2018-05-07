#ifndef FK_FILE_READER_H_INCLUDED
#define FK_FILE_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "file_system.h"

namespace fk {

class FileReader : public lws::SizedReader {
private:
    FkfsStreamingIterator iterator;

public:
    FileReader(FileSystem &fileSystem, uint8_t file);

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;

public:
    void open();
    void open(fkfs_iterator_token_t &token);
    void end();
    void truncate();
    size_t size() override {
        return iterator.size();
    }
    uint8_t fileNumber() {
        return iterator.fileNumber();
    }
    fkfs_iterator_token_t &resumeToken() {
        return iterator.resumeToken();
    }
    bool isFinished() {
        return iterator.isFinished();
    }

};

}

#endif
