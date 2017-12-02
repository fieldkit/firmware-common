#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include "active_object.h"
#include "module_messages.h"

namespace fk {

struct SensorInfo {
    uint8_t sensor;
    const char *name;
    const char *unitOfMeasure;
};

struct ModuleInfo {
    uint8_t address;
    size_t numberOfSensors;
    const char *name;
    SensorInfo *sensors;
};

class HandleIncoming : public Task {
private:
    ModuleInfo *info;
    MessageBuffer &outgoing;
    MessageBuffer &incoming;
    Pool *pool;

public:
    HandleIncoming(ModuleInfo *info, MessageBuffer &o, MessageBuffer &i, Pool &pool);

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
    ModuleInfo *info;

public:
    static Module *active;

public:
    Module(ModuleInfo &info);

public:
    void begin();
    void receive(size_t bytes);
    void reply();

public:
    virtual void beginReading();
    virtual void readingDone();
    virtual void describeSensor(size_t number);

};

}

#endif
