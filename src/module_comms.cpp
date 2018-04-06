#include "module_comms.h"

namespace fk {

ModuleCommunications::ModuleCommunications(TwoWireBus &bus, TaskQueue &queue, Pool &pool) :
    Task("ModuleCommunications"), queue(&queue), bus(&bus), query(pool), reply(pool), twoWireTask("ModuleTwoWire", bus, streams.getReader(), streams.getWriter(), 0) {
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
            streams.clear();

            auto &reader = streams.getReader();
            auto &writer = streams.getWriter();
            auto protoWriter = ProtoBufMessageWriter{ writer };
            protoWriter.write(fk_module_WireMessageQuery_fields, &query.m());

            writer.close();

            twoWireTask = StreamTwoWireTask{ "ModuleTwoWire", *bus, reader, writer, address };
            queue->prepend(twoWireTask);

            hasQuery = false;
            hasReply = false;
        }

        if (twoWireTask.completed()) {
            if (twoWireTask.received() > 0) {
                auto &writer = streams.getWriter();
                auto &reader = streams.getReader();
                auto protoReader = ProtoBufMessageReader{ reader };

                if (!protoReader.read<SERIAL_BUFFER_SIZE>(fk_module_WireMessageReply_fields, reply.forDecode())) {
                    log("Error: Unable to read reply.");
                }
                else {
                    if (reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
                        log("Retry!");
                        twoWireTask = StreamTwoWireTask{ "ModuleTwoWire", *bus, writer, address };
                        queue->prepend(twoWireTask);
                        return TaskEval::idle();
                    }
                    else {
                        hasReply = true;
                    }
                }
            }
            else {
                // log("Error");
            }
            address = 0;
        }
    }

    return TaskEval::idle();
}

ModuleProtocolHandler::ModuleProtocolHandler(ModuleCommunications &communications, Pool &pool) : communications(&communications), pool(&pool) {
}

void ModuleProtocolHandler::push(uint8_t address, ModuleQuery &query, uint32_t delay) {
    active = Queued{ };
    pending = Queued{ address, &query, delay > 0 ? millis() + delay : 0 };
}

bool ModuleProtocolHandler::isBusy() {
    return active || pending;
}

ModuleProtocolHandler::Finished ModuleProtocolHandler::handle() {
    if (!communications->busy()) {
        if (pending.query != nullptr) {
            if (millis() > pending.delay) {
                pool->clear();
                ModuleQueryMessage query{ *pool };
                pending.query->query(query);
                communications->enqueue(pending.address, query);
                active = pending;
                pending = Queued{};
            }
        }
    }

    if (active.query != nullptr) {
        if (communications->available()) {
            ModuleQuery *finished = active.query;
            auto &reply = communications->dequeue();

            active.query->reply(reply);
            active = Queued{};

            return Finished { finished, &reply };
        }
        if (!communications->busy()) {
            ModuleQuery *finished = active.query;

            active = Queued{};

            return Finished { finished, nullptr };
        }
    }

    return Finished{ nullptr, nullptr };
}

}
