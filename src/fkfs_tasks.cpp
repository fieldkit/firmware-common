#include "fkfs_tasks.h"

namespace fk {

DataIteratorTask::DataIteratorTask(fkfs_t &fs) : Task("DataIter"), iterator(fs, 1) {
}

TaskEval DataIteratorTask::task() {
    auto data = iterator.move();
    if (data) {
        // log("Data %d", data.size);
    }

    if (iterator.isFinished()) {
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
