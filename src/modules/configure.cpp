#include "configure.h"
#include "module_idle.h"

namespace fk {

void ConfigureModule::task() {
    transit<ModuleIdle>();
}

}
