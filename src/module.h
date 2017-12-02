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

enum class SensorReadingStatus {
    Idle,
    Busy,
    Done,
};

struct SensorReading {
    uint8_t sensor;
    uint32_t time;
    float value;
    SensorReadingStatus status;
};

struct ModuleInfo {
    uint8_t address;
    size_t numberOfSensors;
    const char *name;
    SensorInfo *sensors;
    SensorReading *readings;
};

// TODO: Rename this
class ModuleCallbacks {
public:
    virtual void beginReading(SensorReading *readings) = 0;

};

class HandleIncoming : public Task {
private:
    ModuleInfo *info;
    ModuleCallbacks *callbacks;
    MessageBuffer &outgoing;
    MessageBuffer &incoming;
    Pool *pool;

public:
    HandleIncoming(ModuleInfo *info, ModuleCallbacks &callbacks, MessageBuffer &o, MessageBuffer &i, Pool &pool);

public:
    void read(size_t bytes);
    TaskEval &task() override;

};

class Module : public ActiveObject, public ModuleCallbacks {
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
    void resume();
    void receive(size_t bytes);
    void reply();

public:
    virtual void beginReading(SensorReading *readings) override;
    virtual void readingDone(SensorReading *readings);

};

}

#endif
