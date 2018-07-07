#ifndef FK_DISCOVERY_H_INCLUDED
#define FK_DISCOVERY_H_INCLUDED

#include "active_object.h"
#include "wifi.h"

namespace fk {

class Discovery : public Task {
private:
    Wifi *wifi;
    uint32_t pingAt{ 0 };

public:
    Discovery(Wifi &wifi);

public:
    TaskEval task() override;

public:
    void ping();

};

}

#endif
