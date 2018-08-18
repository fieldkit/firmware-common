#include "initialized.h"
#include "check_power.h"

namespace fk {

void Initialized::task() {
    transit_into<CheckPower>();
}

}
