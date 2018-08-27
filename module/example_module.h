#ifndef FK_EXAMPLE_MODULE_H_INCLUDED
#define FK_EXAMPLE_MODULE_H_INCLUDED

#include <fk-module-protocol.h>
#include <fk-module.h>

namespace example {

class TakeSensorReadings : public fk::ModuleServicesState {
public:
    const char *name() const override {
        return "TakeSensorReadings";
    }

public:
    void task() override;
};

class ExampleModule : public fk::Module {
private:
    fk::TwoWireBus bus{ fk::Wire4and3 };

public:
    ExampleModule(fk::ModuleInfo &info);

public:
    fk::ModuleStates states() override {
        return {
            fk::ModuleFsm::deferred<fk::ConfigureModule>(),
            fk::ModuleFsm::deferred<TakeSensorReadings>()
        };
    }

};

}

#endif
