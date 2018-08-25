#include "copy_module_data.h"
#include "transmissions.h"
#include "firmware_storage.h"
#include "idle.h"

namespace fk {

void CopyModuleData::task() {
    transit_into<Idle>();
}

}
