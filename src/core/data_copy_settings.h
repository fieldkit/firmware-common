#ifndef FK_DATA_COPY_SETTINGS_H_INCLUDED
#define FK_DATA_COPY_SETTINGS_H_INCLUDED

#include <cinttypes>

namespace fk {

enum class FileNumber : uint8_t {
    System = 0,
    EmergencyLog = 1,
    LogsA = 2,
    LogsB = 3,
    Data = 4,
    None = 0xff
};

struct FileCopySettings {
    FileNumber file{ FileNumber::None };
    uint32_t offset{ 0 };
    uint32_t length{ 0 };
    uint32_t flags{ 0 };

    FileCopySettings() {
    }

    FileCopySettings(FileNumber file) : file(file), offset(0), length(0) {
    }

    FileCopySettings(FileNumber file, uint32_t offset, uint32_t length, uint32_t flags = 0) : file(file), offset(offset), length(length), flags(flags) {
    }
};

}

#endif
