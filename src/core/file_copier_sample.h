#ifndef FK_FILE_COPIER_SAMPLE_H_INCLUDED
#define FK_FILE_COPIER_SAMPLE_H_INCLUDED

#include "task.h"
#include "file_system.h"
#include "core_state.h"

namespace fk {

class FileCopierSample : public Task {
private:
    FileSystem *fileSystem_;
    CoreState *state_;

public:
    FileCopierSample(FileSystem &fileSystem, CoreState &state);

public:
    void enqueued();
    TaskEval task() override;

};

}

#endif
