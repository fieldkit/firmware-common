#include "attached_devices.h"

namespace fk {

AttachedDevices::AttachedDevices(uint8_t *addresses, CoreState &state, Pool &pool)
    : addresses(addresses), state(&state), pool(&pool), queryCapabilities(pool, 0), querySensorCapabilities(pool, 0, 0) {
}

void AttachedDevices::scan() {
    if (addresses[0] > 0) {
        query(addresses[0]);
    }
}

void AttachedDevices::query(uint8_t address) {
    log("[0x%d]: Query", address);

    queryCapabilities = QueryCapabilities(*pool, address);
    push(queryCapabilities);
}

void AttachedDevices::done(Task &task) {
    auto address = addresses[0];
    if (areSame(task, queryCapabilities)) {
        state->merge(address, queryCapabilities.replyMessage());
        querySensorCapabilities = QuerySensorCapabilities(*pool, address, 0);
        push(querySensorCapabilities);
    } else if (areSame(task, querySensorCapabilities)) {
        state->merge(address, querySensorCapabilities.replyMessage());
        uint8_t sensor = querySensorCapabilities.sensor() + 1;
        if (sensor < queryCapabilities.numberOfSensors()) {
            querySensorCapabilities = QuerySensorCapabilities(*pool, address, sensor);
            push(querySensorCapabilities);
        } else {
            addresses++;
            scan();
        }
    }
}

void AttachedDevices::error(Task &task) {
    addresses++;
    scan();
}

}
