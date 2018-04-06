#ifndef FK_TWO_WIRE_TASK_H_INCLUDED
#define FK_TWO_WIRE_TASK_H_INCLUDED

#include "rtc.h"
#include "active_object.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "pool.h"
#include "streams.h"

namespace fk {

class TwoWireTask : public Task {
private:
    TwoWireBus *bus;
    Reader *outgoing;
    Writer *incoming;
    uint8_t address{ 0 };
    uint32_t dieAt{ 0 };
    uint32_t checkAt{ 0 };
    uint32_t doneAt{ 0 };
    size_t bytesReceived{ 0 };

public:
    /**
     *
     */
    TwoWireTask(const char *name, TwoWireBus &bus, Reader &outgoing, Writer &incoming, uint8_t address);

    /**
     *
     */
    TwoWireTask(const char *name, TwoWireBus &bus, Writer &incoming, uint8_t address);

public:
    void enqueued() override;

    TaskEval task() override;

    void error() override {
        doneAt = millis();
    }

    void done() override {
        doneAt = millis();
    }

    bool completed() {
        return doneAt > 0;
    }

    size_t received() {
        return bytesReceived;
    }

private:
    TaskEval send();

    TaskEval receive();

};

}

#include "module_comms.h"

#endif
