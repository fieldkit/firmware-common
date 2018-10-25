#include "factory_reset.h"
#include "startup.h"
#include "core_state.h"
#include "file_system.h"

#include "flash_storage.h"
#include "hardware.h"

namespace fk {

void FactoryReset::task() {
    auto flashFs = services().flashFs;
    auto sdFs = services().fileSystem;
    auto state = services().state;

    Hardware::cyclePeripherals();

    if (!flashFs->initialize(Hardware::FLASH_PIN_CS, SuperBlockSize)) {
        log("Flash unavailable.");
    }
    else {
        log("Flash initialized.");
    }

    state->formatAll();

    sdFs->formatAll();

    transit<StartSystem>();
}

}
