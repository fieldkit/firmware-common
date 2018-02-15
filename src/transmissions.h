#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "i2c.h"
#include "pool.h"

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

class SendTransmission : public ActiveObject {
private:
    MessageBuilder *builder;
    TransmissionTask *method;

public:
    SendTransmission(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

class SendStatus : public ActiveObject {
private:
    MessageBuilder *builder;
    TransmissionTask *method;

public:
    SendStatus(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

}

#endif
