#ifndef FK_ATTACHED_DEVICES_H_INCLUDED
#define FK_ATTACHED_DEVICES_H_INCLUDED

#include "active_object.h"
#include "two_wire_task.h"
#include "core_state.h"

namespace fk {

class Leds;

struct Firmware {
    const char *git;
    const char *build;
};

class QueryCapabilities : public ModuleQuery {
private:
    uint8_t type_{ 0 };
    uint8_t numberOfSensors_{ 0 };
    uint8_t minimumNumberOfReadings_{ 0 };
    Firmware firmware_{ nullptr, nullptr };

public:
    const char *name() const override {
        return "QueryCapabilities";
    }

    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_CAPABILITIES;
    }

    void reply(ModuleReplyMessage &message) override {
        type_ = message.m().capabilities.type;
        numberOfSensors_ = message.m().capabilities.numberOfSensors;
        minimumNumberOfReadings_ = message.m().capabilities.minimumNumberOfReadings;
        firmware_ = {
            (const char *)message.m().capabilities.firmware.git.arg,
            (const char *)message.m().capabilities.firmware.build.arg,
        };
    }

public:
    bool isCommunications() {
        return type_ == fk_module_ModuleType_COMMUNICATIONS;
    }

    bool isSensor() {
        return type_ == fk_module_ModuleType_SENSOR;
    }

    uint8_t getNumberOfSensors() {
        return numberOfSensors_;
    }

    Firmware &firmware() {
        return firmware_;
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
        auto id = message.m().sensorCapabilities.id;
        auto name = (const char *)message.m().sensorCapabilities.name.arg;
        loginfof("QuerySensorCapabilities", "Sensor #%" PRIu32 ": '%s'", id, name);
        sensor++;
    }

    uint8_t getSensor() {
        return sensor;
    }
};

class AttachedDevices : public Task {
private:
    CoreState *state;
    Leds *leds;
    ModuleProtocolHandler protocol;
    uint8_t *addresses{ nullptr };
    uint8_t addressIndex{ 0 };
    uint32_t lastScanAt{ 0 };
    QueryCapabilities queryCapabilities;
    QuerySensorCapabilities querySensorCapabilities;
    uint8_t retries{ 0 };
    bool scanning{ false };

public:
    AttachedDevices(CoreState &state, Leds &leds, ModuleCommunications &communications, uint8_t *addresses);

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
