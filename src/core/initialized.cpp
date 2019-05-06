#include "initialized.h"
#include "check_power.h"
#include "leds.h"
#include "scheduler.h"

namespace fk {

void Initialized::task() {
    TaskLogger logger;
    services().scheduler->accept(logger);

    services().leds->notifyInitialized();

    transit_into<CheckPower>();
}

}
