#ifndef FK_FILE_COPY_OPERATION_H_INCLUDED
#define FK_FILE_COPY_OPERATION_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "data_copy_settings.h"
#include "file_reader.h"
#include "tuning.h"

namespace fk {

class FileCopyCallbacks {
public:
    virtual void fileCopyTick() = 0;
};

class FileCopyOperation {
private:
    lws::BufferedStreamCopier<FileCopyBufferSize> streamCopier_;
    uint32_t started_{ 0 };
    uint32_t status_{ 0 };
    uint32_t copied_{ 0 };
    uint32_t total_{ 0 };
    FileReader reader_;
    bool busy_{ false };

public:
    FileCopyOperation();

public:
    bool prepare(const FileReader &reader, const FileCopySettings &settings);
    bool copy(lws::Writer &writer, FileCopyCallbacks *callbacks = nullptr);

public:
    bool isFinished() const;
    size_t tell();
    size_t size();
    size_t remaining();
    uint32_t version() const;
    uint32_t copied() const;
    uint32_t total() const;

private:
    void status();

};

}

#endif
