#ifndef FK_DISCOVERY_H_INCLUDED
#define FK_DISCOVERY_H_INCLUDED

#include "active_object.h"
#include "wifi.h"

namespace fk {

class Discovery : public Task {
private:
    static constexpr uint16_t FK_CORE_PORT_UDP = 54321;
    uint32_t pingAt{ 0 };

public:
    Discovery();

public:
    void enqueued() override;
    TaskEval task() override;

public:
    void ping();

};

}

#endif
