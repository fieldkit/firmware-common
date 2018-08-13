#ifndef FK_FILE_WRITER_H_INCLUDED
#define FK_FILE_WRITER_H_INCLUDED

#include <lwstreams/lwstreams.h>
#include <phylum/phylum.h>

namespace fk {

class FileWriter : public lws::Writer {
private:
    phylum::BlockedFile *file_{ nullptr };
    bool closed_{ false };

public:
    FileWriter();
    FileWriter(phylum::BlockedFile &file);

public:
    int32_t write(uint8_t *ptr, size_t size) override;
    int32_t write(uint8_t byte) override;
    void close() override;

};

}

#endif
