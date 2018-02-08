#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "core_state.h"
#include "fkfs_tasks.h"

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

class TransmitAllQueuedReadings : public ActiveObject {
private:
    FkfsIterator iterator;
    CoreState *state;
    Pool *pool;

public:
    TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Pool &pool);

public:
    void enqueued();
    TaskEval task() override;

};

}

#endif
