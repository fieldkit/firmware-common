#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

#include "file_reader.h"

namespace fk {

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

    FileReader reader_;

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

    FileReader &reader();

};

}

#endif
