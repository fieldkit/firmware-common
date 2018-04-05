#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "file_system.h"

namespace fk {

class ModuleQuery {
public:
    virtual void query(ModuleQueryMessage &message) = 0;
    virtual void reply(ModuleReplyMessage &message) = 0;

};

class QueryCaps : public ModuleQuery {
private:
    uint8_t numberOfSensors{ 0 };

public:
    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_CAPABILITIES;
    }

    void reply(ModuleReplyMessage &message) override {
        numberOfSensors = message.m().capabilities.numberOfSensors;
    }

public:
    uint8_t getNumberOfSensors() {
        return numberOfSensors;
    }
};

class QuerySensorCaps : public ModuleQuery {
private:
    uint8_t sensor{ 0 };
    
public:
    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_SENSOR_CAPABILITIES;
        message.m().querySensorCapabilities.sensor = sensor;
    }

    void reply(ModuleReplyMessage &message) override {
        sensor++;
    }

    uint8_t getSensor() {
        return sensor;
    }
};

class ModuleProtocolHandler {
private:
    ModuleQuery *active{ nullptr };
    ModuleQuery *pending{ nullptr };
    ModuleCommunications *communications;
    Pool *pool;

public:
    ModuleProtocolHandler(ModuleCommunications &communications, Pool &pool) : communications(&communications), pool(&pool) {
    }

public:
    void push(ModuleQuery &query) {
        active = &query;
    }

    struct Finished {
        ModuleQuery *query;
        ModuleReplyMessage *reply;

        operator bool() {
            return query != nullptr;
        }

        bool error() {
            return reply == nullptr;
        }

        bool is(ModuleQuery &other) {
            return query == &other;
        }
    };

    Finished handle() {
        if (!communications->busy()) {
            if (active != nullptr) {
                pool->clear();
                ModuleQueryMessage query{ *pool };
                active->query(query);
                communications->enqueue(8, query);
                pending = active;
                active = nullptr;
            }
        }

        if (pending != nullptr) {
            if (communications->available()) {
                ModuleQuery *finished = pending;
                auto &reply = communications->dequeue();

                pending->reply(reply);
                pending = nullptr;

                return Finished { finished, &reply };
            }
            if (!communications->busy()) {
                ModuleQuery *finished = pending;

                pending = nullptr;

                return Finished { finished, nullptr };
            }
        }

        return Finished{ nullptr, nullptr };
    }

};

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
    FileSystem *fileSystem;
    FkfsStreamingIterator iterator;
    QueryCaps queryCaps;
    QuerySensorCaps querySensorCaps;
    ModuleProtocolHandler protocol;
    Pool *pool;

public:
    PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications, Pool &pool);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
