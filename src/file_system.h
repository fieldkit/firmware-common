#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include <sd_raw.h>
#include <fkfs.h>
#include <fkfs_log.h>

#include "fkfs_data.h"
#include "fkfs_replies.h"

namespace fk {

class FileSystem {
private:
    static constexpr uint8_t FKFS_FILE_LOG = 0;
    static constexpr uint8_t FKFS_FILE_DATA = 1;
    static constexpr uint8_t FKFS_FILE_PRIORITY_LOWEST = 255;
    static constexpr uint8_t FKFS_FILE_PRIORITY_HIGHEST = 0;

private:
    fkfs_t fs = { 0 };
    fkfs_log_t fkfs_log = { 0 };
    FkfsData data;
    FkfsReplies replies;

public:
    FileSystem(TwoWireBus &bus, TaskQueue &taskQueue, Pool &pool);

public:
    bool setup();

public:
    fkfs_t &fkfs() {
        return fs;
    }

    FkfsData &getData() {
        return data;
    }

    FkfsReplies &getReplies() {
        return replies;
    }

};

}

#endif
