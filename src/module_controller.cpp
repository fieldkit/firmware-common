#include "module_controller.h"

namespace fk {

ModuleController::ModuleController(uint8_t address, Pool &pool)
    : oneSecond(1000), beginTakeReading(pool, address), queryReadingStatus(pool, address) {
}

void ModuleController::beginReading() {
    push(beginTakeReading);
}

void ModuleController::done(Task &task) {
    if (areSame(task, beginTakeReading)) {
        if (!beginTakeReading.isIdle()) {
            if (!beginTakeReading.isDone()) {
                push(oneSecond);
            }
            push(queryReadingStatus);
        } else {
            log("Have readings");
        }
    }

    if (areSame(task, queryReadingStatus)) {
        if (!queryReadingStatus.isIdle()) {
            if (!queryReadingStatus.isDone()) {
                push(oneSecond);
            }
            push(queryReadingStatus);
        } else {
            log("Have readings");
        }
    }
}

void ModuleController::error(Task &) {
}

}
