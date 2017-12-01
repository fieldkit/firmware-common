#ifndef FK_ATTACHED_DEVICES_H_INCLUDED
#define FK_ATTACHED_DEVICES_H_INCLUDED

#include "active_object.h"
#include "two_wire_task.h"

namespace fk {

class AttachedDevices : public ActiveObject {
private:
    uint8_t *addresses;
    uint32_t now;
    Pool *pool;
    QueryCapabilities queryCapabilities;
    QuerySensorCapabilities querySensorCapabilities;

public:
    AttachedDevices(uint8_t *addresses, uint32_t now, Pool *pool);

public:
    void scan();
    void done(Task &task) override;
    void error(Task &task) override;

private:
    void query(uint8_t address, uint32_t now);

};

}

#endif
