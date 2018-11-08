#ifndef FK_DISCOVERY_H_INCLUDED
#define FK_DISCOVERY_H_INCLUDED

#include "task.h"
#include "wifi.h"

namespace fk {

class Discovery : public Task {
private:
    uint32_t scheduled_{ 0 };
    uint8_t packets_{ 0 };

public:
    Discovery();

public:
    TaskEval task() override;

public:
    void ping();

};

}

#endif
