#include "factory_reset_check.h"
#include "startup.h"

namespace fk {

void FactoryResetCheck::task() {
    transit<StartSystem>();
}

}
