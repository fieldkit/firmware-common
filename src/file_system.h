#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include <phylum/phylum.h>
#include <backends/arduino_sd/arduino_sd.h>

#include "fkfs_data.h"
#include "fkfs_replies.h"

namespace fk {

class FileSystem {
private:
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
    phylum::Geometry g{ 0, 4, 4, phylum::SectorSize };
    phylum::ArduinoSdBackend storage;
    phylum::FileLayout<5> fs{ storage };

private:
    Files files_{ fs };
    FkfsData data_;
    FkfsReplies replies_;

public:
    FileSystem(TwoWireBus &bus, Pool &pool);

public:
    bool setup();

public:
    FkfsData &getData() {
        return data_;
    }

    FkfsReplies &getReplies() {
        return replies_;
    }

    Files &files() {
        return files_;
    }

};

}

#endif
