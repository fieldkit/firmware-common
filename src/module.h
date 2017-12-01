#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include "active_object.h"
#include "module_messages.h"

namespace fk {

class HandleIncoming : public Task {
private:
    MessageBuffer &outgoing;
    MessageBuffer &incoming;
    Pool *pool;

public:
    HandleIncoming(MessageBuffer &o, MessageBuffer &i, Pool &pool);

public:
    void read(size_t bytes);
    TaskEval &task() override;

};

class Module : public ActiveObject {
private:
    Pool replyPool;
    MessageBuffer outgoing;
    MessageBuffer incoming;
    HandleIncoming handleIncoming;

public:
    static Module *active;

public:
    Module();
    void begin(uint8_t address);
    void receive(size_t bytes);
    void reply();

};

}

#endif
