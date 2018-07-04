#ifndef FK_FILE_CURSORS_H_INCLUDED
#define FK_FILE_CURSORS_H_INCLUDED

#include "file_system.h"

namespace fk {

struct FileCursor {
    uint32_t time{ 0 };
    uint64_t position{ 0 };
    uint8_t reserved[70];

    FileCursor() {
    }

    FileCursor(uint32_t time, uint64_t position) : time(time), position(position) {
    }
};

struct FileCursors {
    FileCursor cursors[FileSystemNumberOfFiles];
};

class FileCursorManager {
private:
    FileSystem *fileSystem_;

public:
    FileCursorManager(FileSystem &fileSystem);

public:
    bool lookup(FileCursors &cursors);
    bool save(FileCursors cursors);

public:
    uint64_t lookup(FileNumber file);
    bool save(FileNumber file, uint64_t position);

};

}

#endif
