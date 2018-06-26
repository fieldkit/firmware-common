#ifndef FK_FILE_SYSTEM_H_INCLUDED
#define FK_FILE_SYSTEM_H_INCLUDED

#include "fkfs_data.h"
#include "fkfs_replies.h"

namespace fk {

class FileSystem {
private:
    FkfsData data;
    FkfsReplies replies;

public:
    FileSystem(TwoWireBus &bus, Pool &pool);

public:
    bool setup();

public:
    FkfsData &getData() {
        return data;
    }

    FkfsReplies &getReplies() {
        return replies;
    }

};

}

#endif
