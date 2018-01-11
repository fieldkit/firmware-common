#ifndef FK_ATTACHED_DEVICES_H_INCLUDED
#define FK_ATTACHED_DEVICES_H_INCLUDED

#include "active_object.h"
#include "two_wire_task.h"
#include "core_state.h"

namespace fk {

class Leds;

class AttachedDevices : public ActiveObject {
private:
    TwoWireBus *bus;
    uint32_t lastScanAt{ 0 };
    uint8_t *addresses{ nullptr };
    uint8_t addressIndex{ 0 };
    CoreState *state;
    Leds *leds;
    Pool *pool;
    QueryCapabilities queryCapabilities;
    QuerySensorCapabilities querySensorCapabilities;

public:
    AttachedDevices(TwoWireBus &bus, uint8_t *addresses, CoreState &state, Leds &leds, Pool &pool);

public:
    void scan();
    void done(Task &task) override;
    void error(Task &task) override;
    void idle() override;

private:
    void resume();
    void query(uint8_t address);

};

}

#endif
