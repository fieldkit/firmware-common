#include "attached_devices.h"
#include "leds.h"

namespace fk {

AttachedDevices::AttachedDevices(TwoWireBus &bus, uint8_t *addresses, CoreState &state, Leds &leds, Pool &pool)
    : ActiveObject("AttachedDevices"), bus(&bus), addresses(addresses), addressIndex{ 0 }, state(&state), leds(&leds), pool(&pool),
      queryCapabilities(bus, pool, 0), querySensorCapabilities(bus, pool, 0, 0) {
}

void AttachedDevices::scan() {
    addressIndex = 0;
    resume();
}

void AttachedDevices::resume() {
    if (addresses[addressIndex] > 0) {
        query(addresses[addressIndex]);
    } else {
        state->doneScanning();
    }
}

void AttachedDevices::idle() {
    if (state->numberOfModules() == 0) {
        if (lastScanAt == 0 || millis() - lastScanAt > 30 * 1000) {
            log("Starting scan...");
            lastScanAt = millis();
            scan();
        }
    }
}

void AttachedDevices::query(uint8_t address) {
    log("[0x%d]: Query", address);

    queryCapabilities = QueryCapabilities(*bus, *pool, address);
    push(queryCapabilities);
}

void AttachedDevices::done(Task &task) {
    auto address = addresses[addressIndex];
    if (areSame(task, queryCapabilities)) {
        state->merge(address, queryCapabilities.replyMessage());
        querySensorCapabilities = QuerySensorCapabilities(*bus, *pool, address, 0);
        push(querySensorCapabilities);
    } else if (areSame(task, querySensorCapabilities)) {
        state->merge(address, querySensorCapabilities.replyMessage());
        uint8_t sensor = querySensorCapabilities.sensor() + 1;
        if (sensor < queryCapabilities.numberOfSensors()) {
            querySensorCapabilities = QuerySensorCapabilities(*bus, *pool, address, sensor);
            push(querySensorCapabilities);
        } else {
            addressIndex++;
            resume();
        }
    } else {
        if (state->numberOfModules() == 0) {
            leds->noAttachedModules();
        } else {
            leds->haveAttachedModules();
        }
    }
}

void AttachedDevices::error(Task &) {
    addressIndex++;
    resume();
}

}
