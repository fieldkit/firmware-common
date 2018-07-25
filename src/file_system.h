#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include <phylum/phylum.h>
#include <backends/arduino_sd/arduino_sd.h>

#include "fkfs_data.h"
#include "fkfs_replies.h"

namespace fk {

class FileSystem {
private:
    phylum::Geometry g_{ 0, 4, 4, phylum::SectorSize };
    phylum::ArduinoSdBackend storage_;
    phylum::FileLayout<5> fs_{ storage_ };

private:
    Files files_{ fs_ };
    FkfsData data_;
    FkfsReplies replies_;

public:
    FileSystem(TwoWireBus &bus, Pool &pool);

public:
    bool format();
    bool eraseAll();
    bool setup();

public:
    bool beginFileCopy(FileCopySettings settings);
    bool flush();

    bool erase(FileNumber number);

    phylum::SimpleFile openSystem(phylum::OpenMode mode);

    phylum::FileLayout<5> &fs() {
        return fs_;
    }

    FkfsData &getData() {
        return data_;
    }

    FkfsReplies &getReplies() {
        return replies_;
    }

    Files &files() {
        return files_;
    }

private:
    bool closeSystemFiles();
    bool openSystemFiles();

    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));

};

}

#endif
