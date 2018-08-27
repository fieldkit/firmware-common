#ifndef FK_TWO_WIRE_CHILD_H_INCLUDED
#define FK_TWO_WIRE_CHILD_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "pool.h"
#include "two_wire.h"
#include "message_buffer.h"

namespace fk {

class TwoWireChild {
public:
    static TwoWireChild *active_;

private:
    TwoWireBus *bus_;
    uint8_t address_;
    TwoWireMessageBuffer outgoing_;
    TwoWireMessageBuffer incoming_;
    StaticPool<128> replyPool_{ "Reply" };
    lws::CircularStreams<lws::RingBufferN<256>> pipe_;

public:
    TwoWireChild(TwoWireBus &bus, uint8_t address);

public:
    TwoWireMessageBuffer &outgoing() {
        return outgoing_;
    }

    TwoWireMessageBuffer &incoming() {
        return incoming_;
    }

    void clear() {
        pipe_.clear();
    }

public:
    void setup();
    void resume();
    void receive(size_t bytes);
    void reply();

};

}

#endif
