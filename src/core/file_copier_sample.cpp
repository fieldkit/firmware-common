#include "file_copier_sample.h"

namespace fk {

FileCopierSample::FileCopierSample(FileSystem &fileSystem, CoreState &state) : Task("FileCopy"), fileSystem_(&fileSystem), state_(&state) {
}

void FileCopierSample::enqueued() {
    if (!fileSystem_->beginFileCopy({ FileNumber::Data })) {
    }
}

TaskEval FileCopierSample::task() {
    auto &fileCopy = fileSystem_->files().fileCopy();

    if (!fileCopy.isFinished()) {
        auto writer = lws::NullWriter{};
        if (!fileCopy.copy(writer)) {
            return TaskEval::done();
        }
    }
    else {
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
