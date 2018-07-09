#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

#include "tuning.h"
#include "file_reader.h"

namespace fk {

enum class FileNumber {
    System = 0,
    StartupLog = 1,
    NowLog = 2,
    EmergencyLog = 3,
    Data = 4,
};

struct FileCopySettings {
    FileNumber file;
    uint32_t offset;
    uint32_t length;

    FileCopySettings(FileNumber file) : file(file), offset(0), length(0) {
    }

    FileCopySettings(FileNumber file, uint32_t offset, uint32_t length) : file(file), offset(offset), length(length) {
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
    bool busy() const {
        return busy_;
    }
    bool isFinished() {
        return reader_.isFinished();
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
    bool prepare(const FileReader &reader, const FileCopySettings &settings);
    bool copy(lws::Writer &writer, FileCopyCallbacks *callbacks = nullptr);
    bool seek(uint64_t position) {
        return reader_.seek(position);
    }

private:
    void status();

};

class Files {
private:
    phylum::FileDescriptor file_system_area_fd =   { "system",        100 };
    phylum::FileDescriptor file_log_startup_fd =   { "startup.log",   100 };
    phylum::FileDescriptor file_log_now_fd =       { "now.log",       100 };
    phylum::FileDescriptor file_log_emergency_fd = { "emergency.log", 100 };
    phylum::FileDescriptor file_data_fk =          { "data.fk",       0   };
    phylum::FileDescriptor* descriptors_[FileSystemNumberOfFiles]{
        &file_system_area_fd,
        &file_log_startup_fd,
        &file_log_now_fd,
        &file_log_emergency_fd,
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
    phylum::SimpleFile &data();
    phylum::SimpleFile &log();

    FileCopyOperation &fileCopy();

};

}

#endif
