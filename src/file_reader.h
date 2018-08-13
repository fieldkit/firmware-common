#ifndef FK_FILE_READER_H_INCLUDED
#define FK_FILE_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>
#include <phylum/phylum.h>

namespace fk {

class FileReader : public lws::SizedReader {
private:
    phylum::File *file_{ nullptr };
    uint32_t offset_{ 0 };
    uint32_t length_{ 0 };
    bool opened_{ false };
    bool done_{ false };

public:
    FileReader();
    FileReader(phylum::BlockedFile *file);
    FileReader(phylum::SimpleFile *file);

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;
    size_t size() override;
    size_t tell();
    uint32_t version() const;
    bool seek(uint64_t position);

public:
    bool open(uint32_t offset, uint32_t length);
    bool isFinished();
    bool isOpen();

};

}

#endif
