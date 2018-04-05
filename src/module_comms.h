#ifndef FK_MODULE_COMMS_H_INCLUDED
#define FK_MODULE_COMMS_H_INCLUDED

#include "two_wire_task.h"

namespace fk {

class ModuleCommunications : public Task {
private:
    TaskQueue *queue;
    TwoWireBus *bus;
    ModuleQueryMessage query;
    ModuleReplyMessage reply;
    uint8_t address{ 0 };
    bool hasQuery{ false };
    bool hasReply{ false };
    StreamTwoWireTask twoWireTask;
    CircularStreams<fk::RingBufferN<256>> streams;

public:
    ModuleCommunications(TwoWireBus &bus, TaskQueue &queue, Pool &pool);

public:
    TaskEval task() override;

public:
    void enqueue(uint8_t destination, ModuleQueryMessage &pending);

    bool available();

    ModuleReplyMessage &dequeue();

    bool busy() {
        return address > 0;
    }

};

}

#endif
