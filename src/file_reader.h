#ifndef FK_FILE_READER_H_INCLUDED
#define FK_FILE_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>
#include <phylum/phylum.h>

namespace fk {

class FileReader : public lws::SizedReader {
private:
    phylum::SimpleFile *file_{ nullptr };
    bool opened_{ false };
    bool done_{ false };
    size_t size_{ 0 };

public:
    FileReader();
    FileReader(phylum::SimpleFile *file);

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;
    size_t size() override;
    size_t tell();

public:
    void open();
    bool isFinished();
    bool isOpen();

};

}

#endif
