#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

#include "file_reader.h"

namespace fk {

class Files {
private:
    phylum::FileOpener *files;
    phylum::SimpleFile opened;

    phylum::FileDescriptor file_system_area_fd =   { "system",        phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_log_startup_fd =   { "startup.log",   phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_log_now_fd =       { "now.log",       phylum::WriteStrategy::Rolling, 100 };
    phylum::FileDescriptor file_log_emergency_fd = { "emergency.log", phylum::WriteStrategy::Append,  100 };
    phylum::FileDescriptor file_data_fk =          { "data.fk",       phylum::WriteStrategy::Append,  0   };
    phylum::FileDescriptor* descriptors[5]{
        &file_system_area_fd,
        &file_log_startup_fd,
        &file_log_now_fd,
        &file_log_emergency_fd,
        &file_data_fk
    };

    FileReader reader_;

public:
    Files(phylum::FileOpener &files);

    friend class FileSystem;

public:
    phylum::SimpleFile &data();
    phylum::SimpleFile &log();

    FileReader &reader();

};

}

#endif
