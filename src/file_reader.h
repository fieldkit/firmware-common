#ifndef FK_FILE_READER_H_INCLUDED
#define FK_FILE_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "file_system.h"

namespace fk {

class FileReader : public lws::Reader {
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
    uint32_t size() {
        return iterator.size();
    }

};

}

#endif
