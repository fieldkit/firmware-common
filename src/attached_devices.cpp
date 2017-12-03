#include "attached_devices.h"

namespace fk {

AttachedDevices::AttachedDevices(uint8_t *addresses, uint32_t now, Pool &pool)
    : addresses(addresses), now(now), pool(&pool), queryCapabilities(pool, 0, 0), querySensorCapabilities(pool, 0, 0) {
}

void AttachedDevices::scan() {
    if (addresses[0] > 0) {
        query(addresses[0], now);
    }
}

void AttachedDevices::query(uint8_t address, uint32_t now) {
    log("[0x%d]: Query", address);

    queryCapabilities = QueryCapabilities(*pool, address, now);
    push(queryCapabilities);
}

void AttachedDevices::done(Task &task) {
    if (areSame(task, queryCapabilities)) {
        querySensorCapabilities = QuerySensorCapabilities(*pool, addresses[0], 0);
        push(querySensorCapabilities);
    } else if (areSame(task, querySensorCapabilities)) {
        uint8_t sensor = querySensorCapabilities.sensor() + 1;
        if (sensor < queryCapabilities.numberOfSensors()) {
            querySensorCapabilities = QuerySensorCapabilities(*pool, addresses[0], sensor);
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
