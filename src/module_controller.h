#ifndef FK_MODULE_CONTROLLER_H_INCLUDED
#define FK_MODULE_CONTROLLER_H_INCLUDED

#include "active_object.h"
#include "two_wire_task.h"

namespace fk {

class ModuleController : public ActiveObject {
private:
    Delay oneSecond;
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;

public:
    ModuleController(uint8_t address, Pool *pool);

public:
    void done(Task &task) override;
    void error(Task &task) override;

public:
    void beginReading();

};

}

#endif
