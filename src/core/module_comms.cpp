#include "module_comms.h"
#include "tuning.h"

namespace fk {

void ModuleQuery::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    query(message);

    auto protoWriter = lws::ProtoBufMessageWriter{ outgoing };
    protoWriter.write(fk_module_WireMessageQuery_fields, message.forEncode());
    outgoing.close();
}

void ModuleQuery::tick(lws::Writer &outgoing) {
}

ModuleCommunications::ModuleCommunications(TwoWireBus &bus, Pool &pool) :
    Task("ModuleCommunications"), bus(&bus), pool(&pool), query(pool), reply(pool), twoWireTask("ModuleTwoWire", bus, outgoing.getReader(), incoming.getWriter(), 0, ReplyConfig::Default) {
}

void ModuleCommunications::enqueue(uint8_t destination, ModuleQuery &mq) {
    fk_assert(!hasQuery);

    address = destination;
    pending = &mq;
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
    TwoWireStatistics tws;
    return task(tws);
}

TaskEval ModuleCommunications::task(TwoWireStatistics &tws) {
    if (address > 0) {
        auto replyConfig = pending->replyConfig();

        if (hasQuery) {
            incoming.clear();
            outgoing.clear();
            query.clear();

            pending->prepare(query, outgoing.getWriter());

            twoWireTask = TwoWireTask{ pending->name(), *bus,
                                       outgoing.getReader(), incoming.getWriter(),
                                       address, replyConfig };
            twoWireTask.enqueued();

            hasQuery = false;
            hasReply = false;
            started = fk_uptime();
        }
        else {
            simple_task_run(twoWireTask);

            pending->tick(outgoing.getWriter());
        }

        auto elapsed = fk_uptime() - started;

        if (twoWireTask.completed()) {
            tws.expected += replyConfig.expected_replies;

            if (replyConfig.expected_replies == 0) {
                log("No reply expected");
                address = 0;
                return TaskEval::idle();
            }

            if (twoWireTask.received() > 0) {
                auto protoReader = lws::ProtoBufMessageReader{ incoming.getReader() };

                if (!protoReader.read<SERIAL_BUFFER_SIZE>(fk_module_WireMessageReply_fields, reply.forDecode())) {
                    log("Error: Unable to read reply.");
                    tws.malformed++;
                }
                else {
                    if (reply.m().type == fk_module_ReplyType_REPLY_BUSY || reply.m().type == fk_module_ReplyType_REPLY_RETRY) {
                        incoming.clear();
                        outgoing.clear();
                        query.clear();

                        if (reply.m().type == fk_module_ReplyType_REPLY_BUSY) {
                            twoWireTask = TwoWireTask{ pending->name(), *bus, incoming.getWriter(), address, replyConfig };
                            twoWireTask.enqueued();
                            log("Busy (%lums)", elapsed);
                            tws.busy++;
                        }
                        else {
                            hasQuery = true;
                            hasReply = false;
                            log("Retry (%lums)", elapsed);
                            tws.retry++;
                        }

                        return TaskEval::idle();
                    }
                    else {
                        hasReply = true;
                        tws.reply++;
                    }
                }
            }
            else {
                tws.missed++;
            }
            address = 0;
        }

        if (elapsed > replyConfig.transaction_timeout) {
            log("Timeout!");
            tws.timeouts++;
            address = 0;
            return TaskEval::error();
        }
    }

    return TaskEval::idle();
}

ModuleProtocolHandler::ModuleProtocolHandler(ModuleCommunications &communications) : communications(&communications) {
}

void ModuleProtocolHandler::push(uint8_t address, ModuleQuery &query, uint32_t delay) {
    active = Queued{ };
    pending = Queued{ address, &query, delay > 0 ? fk_uptime() + delay : 0 };
}

bool ModuleProtocolHandler::isBusy() {
    return active || pending;
}

ModuleProtocolHandler::Finished ModuleProtocolHandler::handle() {
    if (!communications->busy()) {
        if (pending.query != nullptr) {
            if (fk_uptime() > pending.delay) {
                communications->enqueue(pending.address, *pending.query);
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
