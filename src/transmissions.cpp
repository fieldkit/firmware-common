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

}
