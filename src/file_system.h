#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include <phylum/phylum.h>
#include <backends/arduino_sd/arduino_sd.h>

#include "fkfs_data.h"
#include "fkfs_replies.h"

namespace fk {

class FileSystem {
private:
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
    bool openData();

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
