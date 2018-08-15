#include <module_idle.h>

#include "example_module.h"

namespace example {

class TakeSensorReadings : public fk::ModuleServicesState {
public:
    const char *name() const override {
        return "TakeSensorReadings";
    }

public:
    void task() override;
};

void TakeSensorReadings::task() {
    transit<fk::ModuleIdle>();
}

ExampleModule::ExampleModule(fk::ModuleInfo &info) :
    Module(bus, info) {
}

fk::ModuleReadingStatus ExampleModule::beginReading(fk::PendingSensorReading &pending) {
    log("Readings!");

    auto readings = pending.readings;
    for (size_t i = 0; i < 3; ++i) {
        readings[i].time = fk::clock.getTime();
        readings[i].value = random(10, 20);
        readings[i].status = fk::SensorReadingStatus::Done;
    }

    return fk::ModuleReadingStatus();
}

fk::DeferredModuleState ExampleModule::beginReadingState() {
    return fk::ModuleFsm::deferred<TakeSensorReadings>();
}

}
