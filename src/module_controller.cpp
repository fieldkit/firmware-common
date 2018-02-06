#include "module_controller.h"

namespace fk {

ModuleController::ModuleController(TwoWireBus &bus, uint8_t address, Pool &pool)
    : ActiveObject("Module"), oneSecond(1000), beginTakeReading(bus, pool, address), queryReadingStatus(bus, pool, address) {
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
