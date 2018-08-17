#include "initialized.h"
#include "check_power.h"
#include "copy_module_data.h"

namespace fk {

void Initialized::task() {
    transit_into<CopyModuleData>();
}

}
