#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

namespace fk {

class MessageBuilder {
public:
    virtual bool write(Print &stream) = 0;
    virtual const char *getContentType() = 0;

};

class TransmissionTask : public ActiveObject {
private:
    MessageBuilder *builder{ nullptr };

public:
    TransmissionTask(const char *name) : ActiveObject(name) {
    }

public:
    void prepare(MessageBuilder &mb) {
        builder = &mb;
    }

public:
    void write(Print &stream) {
        if (!builder->write(stream)) {
            log("Error writing message.");
        }
    }

    const char *getContentType() {
        return builder->getContentType();
    }

};

}

#endif
