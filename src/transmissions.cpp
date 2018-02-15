#include "transmissions.h"

namespace fk {

TransmissionTask::TransmissionTask(const char *name) : ActiveObject(name) {
}

void TransmissionTask::prepare(MessageBuilder &mb) {
    builder = &mb;
}

void TransmissionTask::write(Print &stream) {
    if (!builder->write(stream)) {
        log("Error writing message.");
    }
}

const char *TransmissionTask::getContentType() {
    return builder->getContentType();
}

SendTransmission::SendTransmission(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendTransmission"), builder(&builder), method(&method) {
}

void SendTransmission::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendTransmission::done(Task &task) {
}

SendStatus::SendStatus(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendStatus"), builder(&builder), method(&method) {
}

void SendStatus::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendStatus::done(Task &task) {
}

}
