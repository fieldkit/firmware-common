#ifndef FK_EXAMPLE_MODULE_H_INCLUDED
#define FK_EXAMPLE_MODULE_H_INCLUDED

#include <fk-module-protocol.h>
#include <fk-module.h>

namespace example {

class ExampleModule : public fk::Module {
private:
    fk::TwoWireBus bus{ fk::Wire4and3 };

public:
    ExampleModule(fk::ModuleInfo &info);

public:
    fk::ModuleReadingStatus beginReading(fk::PendingSensorReading &pending) override;
    fk::Deferred beginReadingState() override;

};

}

#endif
