#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

namespace fk {

class MessageBuilder {
public:
    virtual bool isEmpty() = 0;
    virtual bool write(Print &stream) = 0;
    virtual const char *getContentType() = 0;

};

class TransmissionTask : public ActiveObject {
private:
    MessageBuilder *builder{ nullptr };

public:
    TransmissionTask(const char *name);

public:
    void prepare(MessageBuilder &mb);

public:
    bool isEmpty() {
        return builder->isEmpty();
    }
    void write(Print &stream);
    const char *getContentType();

};

}

#endif
