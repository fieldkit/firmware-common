#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

#include "tuning.h"
#include "file_reader.h"

namespace fk {

enum class FileNumber {
    System = 0,
    EmergencyLog = 1,
    LogsA = 2,
    LogsB = 3,
    Data = 4
};

struct FileCopySettings {
    FileNumber file;
    uint32_t offset;
    uint32_t length;
    uint32_t flags;

    FileCopySettings(FileNumber file) : file(file), offset(0), length(0) {
    }

    FileCopySettings(FileNumber file, uint32_t offset, uint32_t length, uint32_t flags = 0) : file(file), offset(offset), length(length), flags(flags) {
    }
};

class FileCopyCallbacks {
public:
    virtual void fileCopyTick() = 0;
};

class FileCopyOperation {
private:
    lws::BufferedStreamCopier<FileCopyBufferSize> streamCopier_;
    uint32_t started_{ 0 };
    uint32_t lastStatus_{ 0 };
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
    bool isFinished() const {
        return !busy_;
    }
    size_t tell() {
        return reader_.tell();
    }
    size_t size() {
        return reader_.size();
    }
    size_t remaining() {
        return size() - tell();
    }
    size_t copied() const {
        return copied_;
    }
    uint32_t version() const {
        return reader_.version();
    }
    bool seek(uint64_t position) {
        return reader_.seek(position);
    }

private:
    void status();

};

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

    phylum::SimpleFile &data();

    phylum::SimpleFile &log();

    FileNumber logFileNumber();

    FileCopyOperation &fileCopy();

};

}

#endif
