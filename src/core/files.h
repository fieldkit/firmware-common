#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

#include "tuning.h"
#include "file_reader.h"
#include "file_copy_operation.h"

namespace fk {

class Files {
private:
    phylum::FileDescriptor file_system_area_fd = { "system",          100  };
    phylum::FileDescriptor file_emergency_fd   = { "emergency.fklog", 100  };
    phylum::FileDescriptor file_logs_a_fd =      { "logs-a.fklog",    2048 };
    phylum::FileDescriptor file_logs_b_fd =      { "logs-b.fklog",    2048 };
    phylum::FileDescriptor file_data_fk =        { "data.fk",         0    };
    phylum::FileDescriptor* descriptors_[FileSystemNumberOfFiles]{
        &file_system_area_fd,
        &file_emergency_fd,
        &file_logs_a_fd,
        &file_logs_b_fd,
        &file_data_fk
    };

private:
    phylum::SimpleFile opened_;
    phylum::SimpleFile log_;
    phylum::SimpleFile data_;
    phylum::FileOpener *files_;
    FileCopyOperation fileCopy_;
    uint8_t errors_{ 0 };

public:
    Files(phylum::FileOpener &files);

    friend class FileSystem;

public:
    bool isInternal(phylum::FileDescriptor &fd) const {
        return &fd == &file_system_area_fd;
    }

    size_t numberOfFiles() const {
        return FileSystemNumberOfFiles;
    }

    phylum::FileDescriptor &file(size_t number) const {
        return *descriptors_[number];
    }

public:
    bool swapLogsIfNecessary();

    void error();

    bool errors() const;

    phylum::SimpleFile &data();

    phylum::SimpleFile &log();

    FileNumber logFileNumber();

    FileCopyOperation &fileCopy();

};

}

#endif
