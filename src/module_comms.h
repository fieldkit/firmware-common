#ifndef FK_MODULE_COMMS_H_INCLUDED
#define FK_MODULE_COMMS_H_INCLUDED

#include "two_wire_task.h"

namespace fk {

class ModuleQuery {
public:
    virtual const char *name() const = 0;
    virtual void query(ModuleQueryMessage &message) = 0;
    virtual void reply(ModuleReplyMessage &message) = 0;

};

class ModuleCommunications : public Task {
private:
    TaskQueue *queue;
    TwoWireBus *bus;
    ModuleQueryMessage query;
    ModuleReplyMessage reply;
    uint8_t address{ 0 };
    bool hasQuery{ false };
    bool hasReply{ false };
    TwoWireTask twoWireTask;
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

class ModuleProtocolHandler {
public:
    struct Queued {
        uint8_t address{ 0 };
        ModuleQuery *query{ nullptr };
        uint32_t delay{ 0 };

        Queued() {
        }

        Queued(uint8_t address, ModuleQuery *query, uint32_t delay) : address(address), query(query), delay(delay) {
        }

        operator bool() {
            return address > 0;
        }
    };

    struct Finished {
        ModuleQuery *query;
        ModuleReplyMessage *reply;

        operator bool() {
            return query != nullptr;
        }

        bool error() {
            return reply == nullptr;
        }

        bool is(ModuleQuery &other) {
            return query == &other;
        }
    };

private:
    Queued active;
    Queued pending;
    ModuleCommunications *communications;
    Pool *pool;

public:
    ModuleProtocolHandler(ModuleCommunications &communications, Pool &pool);

public:
    void push(uint8_t address, ModuleQuery &query, uint32_t delay = 0);

    bool isBusy();

    Finished handle();

};

}

#endif
