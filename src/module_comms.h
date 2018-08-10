#ifndef FK_MODULE_COMMS_H_INCLUDED
#define FK_MODULE_COMMS_H_INCLUDED

#define LWS_ENABLE_PROTOBUF

#include <lwstreams/lwstreams.h>

#include "two_wire_task.h"

namespace fk {

class ModuleQuery {
public:
    virtual const char *name() const = 0;
    virtual void query(ModuleQueryMessage &message) = 0;
    virtual void reply(ModuleReplyMessage &message) = 0;
    virtual void prepare(ModuleQueryMessage &message, lws::Writer &outgoing);
    virtual void tick(lws::Writer &outgoing);
    virtual bool replyExpected() {
        return true;
    }

};

class ModuleCommunications : public Task {
private:
    TwoWireBus *bus;
    Pool *pool;
    uint8_t address{ 0 };
    ModuleQuery *pending{ nullptr };
    ModuleQueryMessage query;
    ModuleReplyMessage reply;
    bool hasQuery{ false };
    bool hasReply{ false };
    TwoWireTask twoWireTask;
    lws::CircularStreams<lws::RingBufferN<256>> outgoing;
    lws::CircularStreams<lws::RingBufferN<256>> incoming;

public:
    ModuleCommunications(TwoWireBus &bus, Pool &pool);

public:
    TaskEval task() override;

public:
    void enqueue(uint8_t destination, ModuleQuery &mq);

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

public:
    ModuleProtocolHandler(ModuleCommunications &communications);

public:
    void push(uint8_t address, ModuleQuery &query, uint32_t delay = 0);

    bool isBusy();

    Finished handle();

};

}

#endif
