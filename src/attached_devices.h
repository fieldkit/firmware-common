#ifndef FK_ATTACHED_DEVICES_H_INCLUDED
#define FK_ATTACHED_DEVICES_H_INCLUDED

#include "active_object.h"
#include "two_wire_task.h"
#include "core_state.h"

namespace fk {

class Leds;

class QueryCapabilities : public ModuleQuery {
private:
    uint8_t type{ 0 };
    uint8_t numberOfSensors{ 0 };

public:
    const char *name() const override {
        return "QueryCapabilities";
    }

    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_CAPABILITIES;
    }

    void reply(ModuleReplyMessage &message) override {
        type = message.m().capabilities.type;
        numberOfSensors = message.m().capabilities.numberOfSensors;
    }

public:
    bool isCommunications() {
        return type == fk_module_ModuleType_COMMUNICATIONS;
    }

    bool isSensor() {
        return type == fk_module_ModuleType_SENSOR;
    }

    uint8_t getNumberOfSensors() {
        return numberOfSensors;
    }
};

class QuerySensorCapabilities : public ModuleQuery {
private:
    uint8_t sensor{ 0 };

public:
    const char *name() const override {
        return "QuerySensorCapabilities";
    }

    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_SENSOR_CAPABILITIES;
        message.m().querySensorCapabilities.sensor = sensor;
    }

    void reply(ModuleReplyMessage &message) override {
        sensor++;
        loginfof("QuerySensorCapabilities", "Sensor #%" PRIu32 ": '%s'", message.m().sensorCapabilities.id, (const char *)message.m().sensorCapabilities.name.arg);
    }

    uint8_t getSensor() {
        return sensor;
    }
};

class AttachedDevices : public Task {
private:
    TwoWireBus *bus;
    uint32_t lastScanAt{ 0 };
    uint8_t *addresses{ nullptr };
    uint8_t addressIndex{ 0 };
    CoreState *state;
    Leds *leds;
    ModuleProtocolHandler protocol;
    QueryCapabilities queryCapabilities;
    QuerySensorCapabilities querySensorCapabilities;
    uint8_t retries{ 0 };
    bool scanning{ false };

public:
    AttachedDevices(TwoWireBus &bus, uint8_t *addresses, CoreState &state, Leds &leds, ModuleCommunications &communications);

public:
    TaskEval task() override;

public:
    void scan();
    void done(ModuleProtocolHandler::Finished &task);
    void error(ModuleProtocolHandler::Finished &task);

private:
    void resume();
    void query(uint8_t address);

};

}

#endif
