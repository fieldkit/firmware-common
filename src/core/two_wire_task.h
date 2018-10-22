#ifndef FK_TWO_WIRE_TASK_H_INCLUDED
#define FK_TWO_WIRE_TASK_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "rtc.h"
#include "task.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "pool.h"
#include "tuning.h"

namespace fk {

struct ReplyConfig {
    /***
     * Default timeout durations for standard transactions.
     */
    static constexpr uint32_t TwoWireDefaultTimeout = 5 * Seconds;

    /***
     * Default timeout durations for standard transactions.
     *
     * This is high because we retry fast and some operations, like DATA_PREPARE
     * can take a few seconds. Most other operations will require a smaller
     * number of retries, so maybe this is kind of ugly.
     */
    static constexpr uint32_t TwoWireLongTimeout = 30 * Seconds;

    /**
     * Number of replies to expect from the module.
     */
    uint8_t expected_replies{ 1 };

    /**
     * How long to wait before asking for a reply after being told their busy.
     */
    uint32_t busy_delay{ 500 };

    /**
     * How long to wait before asking for a reply.
     */
    uint32_t reply_delay{ 100 };

    /**
     * Maximum time to wait for any kind of reply, including busy replies.
     */
    uint32_t reply_timeout{ 1 * Seconds };

    /**
     * Maximum time to wait for an entire transaction, from query to non-busy reply.
     */
    uint32_t transaction_timeout{ 5 * Seconds };

private:
    ReplyConfig() {
    }

    ReplyConfig(bool expected) : expected_replies(expected ? 1 : 0) {
    }

    ReplyConfig(uint32_t transaction_timeout) : transaction_timeout(transaction_timeout) {
    }

public:
    static ReplyConfig Default;
    static ReplyConfig NoReply;
    static ReplyConfig Readings;
    static ReplyConfig Long;
};

class TwoWireTask : public Task {
private:
    TwoWireBus *bus;
    lws::Reader *outgoing;
    lws::Writer *incoming;
    uint8_t address{ 0 };
    uint32_t dieAt{ 0 };
    uint32_t checkAt{ 0 };
    uint32_t doneAt{ 0 };
    uint32_t bytesSent{ 0 };
    ReplyConfig replyConfig;
    size_t bytesReceived{ 0 };
    int8_t repliesRemaining{ 0 };

public:
    TwoWireTask(const char *name, TwoWireBus &bus, lws::Reader &outgoing, lws::Writer &incoming, uint8_t address, ReplyConfig replyConfig);
    TwoWireTask(const char *name, TwoWireBus &bus, lws::Writer &incoming, uint8_t address, ReplyConfig replyConfig);

public:
    void enqueued() override;

    TaskEval task() override;

    void error() override {
        doneAt = fk_uptime();
    }

    void done() override {
        doneAt = fk_uptime();
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

#endif
