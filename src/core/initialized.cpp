#include "initialized.h"
#include "check_power.h"
#include "leds.h"

namespace fk {

void Initialized::task() {
    services().leds->notifyInitialized();

    transit_into<CheckPower>();
}

}
