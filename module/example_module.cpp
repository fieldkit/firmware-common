#include "example_module.h"

namespace example {

TakeFakeReadings::TakeFakeReadings() : Task("TakeFakeReadings") {
}

TakeFakeReadings &TakeFakeReadings::into(fk::SensorReading *r) {
    readings = r;
    return *this;
}

fk::TaskEval TakeFakeReadings::task() {
    for (size_t i = 0; i < 3; ++i) {
        readings[i].time = fk::clock.getTime();
        readings[i].value = random(10, 20);
        readings[i].status = fk::SensorReadingStatus::Done;
    }

    return fk::TaskEval::done();
}

ExampleModule::ExampleModule(fk::ModuleInfo &info, Sensors &sensors) :
    Module(bus, info), sensors(sensors) {
}

fk::ModuleReadingStatus ExampleModule::beginReading(fk::PendingSensorReading &pending) {
    log("Readings!");

    sensors.push(takeFakeReadings.into(pending.readings));

    return fk::ModuleReadingStatus();
}

}
