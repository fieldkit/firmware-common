#ifndef FK_FILE_READER_H_INCLUDED
#define FK_FILE_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>

namespace fk {

class FileReader : public lws::SizedReader {
private:

public:
    FileReader();

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;

public:
    void open();
    void end();
    void truncate();
    size_t size() override {
        return 0;
    }
    bool isFinished() {
        return false;
    }

};

}

#endif
