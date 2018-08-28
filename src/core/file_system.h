#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include <phylum/phylum.h>
#include <backends/arduino_sd/arduino_sd.h>

#include "data_logging.h"
#include "data_replies.h"

namespace fk {

class FileSystem {
private:
    phylum::Geometry g_{ 0, 4, 4, phylum::SectorSize };
    phylum::ArduinoSdBackend storage_;
    phylum::FileLayout<5> fs_{ storage_ };

private:
    Files files_{ fs_ };
    DataLogging data_;
    DataReplies replies_;

public:
    FileSystem();

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

    DataLogging &getData() {
        return data_;
    }

    DataReplies &getReplies() {
        return replies_;
    }

    Files &files() {
        return files_;
    }

private:
    bool closeSystemFiles();
    bool openSystemFiles();

};

}

#endif
