#include "file_cursors.h"
#include "rtc.h"

namespace fk {

constexpr const char LogName[] = "FCM";

using Logger = SimpleLog<LogName>;

FileCursorManager::FileCursorManager(FileSystem &fileSystem) : fileSystem_(&fileSystem) {
}

bool FileCursorManager::lookup(FileCursors &cursors) {
    auto file = fileSystem_->openSystem(phylum::OpenMode::Read);
    auto size = file.size();

    if (size < sizeof(FileCursors)) {
        cursors = { };
        return true;
    }

    auto position = file.size() - sizeof(FileCursors);
    if (!file.seek(position)) {
        Logger::warn("Unable to seek");
        return false;
    }

    if (file.read((uint8_t *)&cursors, sizeof(FileCursors)) != sizeof(FileCursors)) {
        Logger::warn("Unable to read");
        return false;
    }

    return true;
}

bool FileCursorManager::save(FileCursors cursors) {
    auto file = fileSystem_->openSystem(phylum::OpenMode::Write);

    auto status = file.write((uint8_t *)&cursors, sizeof(FileCursors));

    file.close();

    return status;
}

uint64_t FileCursorManager::lookup(FileNumber file) {
    FileCursors cursors;

    fk_assert((size_t)file < FileSystemNumberOfFiles);

    if (!lookup(cursors)) {
        return 0;
    }

    return cursors.cursors[(size_t)file].position;
}

bool FileCursorManager::save(FileNumber file, uint64_t position) {
    FileCursors cursors;

    fk_assert((size_t)file < FileSystemNumberOfFiles);

    if (!lookup(cursors)) {
        return false;
    }

    cursors.cursors[(size_t)file] = { clock.getTime(), position };

    if (!save(cursors)) {
        return false;
    }

    return true;
}


}
