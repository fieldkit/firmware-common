#include "module_comms.h"

namespace fk {

ModuleCommunications::ModuleCommunications(TwoWireBus &bus, TaskQueue &queue, Pool &pool) :
    Task("ModuleCommunications"), queue(&queue), bus(&bus), query(pool), reply(pool), twoWireTask("ModuleTwoWire", bus, outgoing, incoming, 0) {
}

void ModuleCommunications::enqueue(uint8_t destination, ModuleQueryMessage &pending) {
    fk_assert(!hasQuery);

    address = destination;
    query = pending;
    hasQuery = true;
}

bool ModuleCommunications::available() {
    return hasReply;
}

ModuleReplyMessage &ModuleCommunications::dequeue() {
    fk_assert(hasReply);
    hasReply = false;
    return reply;
}

TaskEval ModuleCommunications::task() {
    if (address > 0) {
        if (hasQuery) {
            auto pbWriter = ProtoBufMessageWriter{ buffer.toBufferPtr() };
            pbWriter.write(fk_module_WireMessageQuery_fields, &query.m());

            auto bufferPtr = pbWriter.toBufferPtr();
            outgoing = DirectReader { bufferPtr };
            incoming = DirectWriter { buffer.toBufferPtr() };

            twoWireTask = StreamTwoWireTask{ "ModuleTwoWire", *bus, outgoing, incoming, address };

            queue->prepend(twoWireTask);

            hasQuery = false;
            hasReply = false;
        }

        if (twoWireTask.completed()) {
            if (twoWireTask.received() > 0) {
                auto received = twoWireTask.received();
                auto bufferPtr = incoming.toBufferPtr();
                auto messageBuffer = DirectMessageBuffer{ bufferPtr.ptr, received };

                messageBuffer.end();

                if (!messageBuffer.read(reply)) {
                    log("Error: Unable to read reply.");
                }
                else {
                    if (reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
                        log("Retry!");
                        twoWireTask = StreamTwoWireTask{ "ModuleTwoWire", *bus, outgoing, incoming, address };
                    }
                    else {
                        log("Reply!");
                        hasReply = true;
                    }
                }
            }
            else {
                log("Error");
            }
            address = 0;
        }
    }

    return TaskEval::idle();
}

}
