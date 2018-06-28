#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

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
};

class FileCopyOperation {
private:
    lws::BufferedStreamCopier<512> streamCopier_;
    uint32_t started_{ 0 };
    uint32_t lastStatus_{ 0 };
    uint32_t copied_{ 0 };
    FileReader reader_;

public:
    FileCopyOperation();

public:
    bool isFinished() {
        return reader_.isFinished();
    }
    size_t tell() {
        return reader_.tell();
    }
    size_t size() {
        return reader_.size();
    }
    size_t copied() {
        return copied_;
    }
    bool prepare(FileReader reader);
    bool tick(lws::Writer &writer);

private:
    void status();

};

class Files {
private:
    static constexpr size_t NumberOfFiles = 5;

    phylum::FileDescriptor file_system_area_fd =   { "system",        phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_log_startup_fd =   { "startup.log",   phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_log_now_fd =       { "now.log",       phylum::WriteStrategy::Rolling, 100 };
    phylum::FileDescriptor file_log_emergency_fd = { "emergency.log", phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_data_fk =          { "data.fk",       phylum::WriteStrategy::Append,  0   };
    phylum::FileDescriptor* descriptors_[NumberOfFiles]{
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
    size_t numberOfFiles() const {
        return NumberOfFiles;
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
