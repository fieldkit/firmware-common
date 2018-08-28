#include <module_idle.h>

#include "example_module.h"

namespace example {

void TakeSensorReadings::task() {
    for (size_t i = 0; i < 3; ++i) {
        services().readings->done(i, random(10, 20));
    }

    transit<fk::ModuleIdle>();
}

ExampleModule::ExampleModule(fk::ModuleInfo &info) :
    Module(bus, info, { 6 }) {
}

}
