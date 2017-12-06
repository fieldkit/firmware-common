#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"

namespace fk {

constexpr uint32_t MaximumTransmissionLength = 128;

class TransmissionTask : public ActiveObject {
private:
    char buffer[MaximumTransmissionLength];

public:
    TransmissionTask(const char *name) : ActiveObject(name) {
        buffer[0] = 0;
    }

public:
    void prepare(const char *message) {
        strncpy(buffer, message, sizeof(buffer));
    }

};

}

#endif
