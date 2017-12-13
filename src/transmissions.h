#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

namespace fk {

class MessageBuilder {
public:
    virtual bool write(Stream &stream) = 0;

};

class JsonMessageBuilder : public MessageBuilder {
private:
    CoreState *state;

public:
    JsonMessageBuilder(CoreState &state) : state(&state) {
    }

public:
    bool write(Stream &stream) override;

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

    void write(Stream &stream) {
        if (!builder->write(stream)) {
            log("Error writing message.");
        }
    }

};

}

#endif
