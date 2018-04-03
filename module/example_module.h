#ifndef FK_EXAMPLE_MODULE_H_INCLUDED
#define FK_EXAMPLE_MODULE_H_INCLUDED

#include <fk-module-protocol.h>
#include <fk-module.h>

namespace example {

class TakeFakeReadings : public fk::Task {
private:
    fk::SensorReading *readings;

public:
    TakeFakeReadings();

public:
    TakeFakeReadings &into(fk::SensorReading *r);
    fk::TaskEval task() override;

};

class Sensors : public fk::ActiveObject {
public:
    Sensors() : fk::ActiveObject("Sensors") {
    }
};

class ExampleModule : public fk::Module {
private:
    fk::TwoWireBus bus{ Wire };
    fk::Delay fiveSeconds{ 5000 };
    TakeFakeReadings takeFakeReadings;
    Sensors &sensors;

public:
    ExampleModule(fk::ModuleInfo &info, Sensors &sensors);

public:
    virtual fk::ModuleReadingStatus beginReading(fk::PendingSensorReading &pending) override;

};

}

#endif
