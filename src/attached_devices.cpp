#include "attached_devices.h"
#include "leds.h"
#include "tuning.h"

namespace fk {

AttachedDevices::AttachedDevices(TwoWireBus &bus, uint8_t *addresses, CoreState &state, Leds &leds, Pool &pool)
    : ActiveObject("AttachedDevices"), bus(&bus), addresses(addresses), addressIndex{ 0 }, state(&state), leds(&leds), pool(&pool),
      queryCapabilities(bus, pool, 0), querySensorCapabilities(bus, pool, 0, 0) {
}

void AttachedDevices::scan() {
    if (peripherals.twoWire1().tryAcquire(this)) {
        addressIndex = 0;
        resume();
    }
}

void AttachedDevices::resume() {
    while (true) {
        auto address = addresses[addressIndex];
        if (address > 0) {
            if (!state->hasModules() || state->hasModuleWithAddress(address)) {
                query(address);
                break;
            }
        } else {
            state->doneScanning();
            peripherals.twoWire1().release(this);
            break;
        }

        addressIndex++;
    }
}

void AttachedDevices::idle() {
    if (state->isBusy()) {
        return;
    }

    auto rescanInterval = (state->numberOfModules() == 0 ? RescanExistingModulesInterval : RescanExistingModulesInterval);
    if (lastScanAt == 0 || millis() - lastScanAt > rescanInterval) {
        log("Starting scan...");
        lastScanAt = millis();
        pool->clear();
        scan();
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
        if (queryCapabilities.isSensor()) {
            log("[0x%d]: Sensor module", address);
            querySensorCapabilities = QuerySensorCapabilities(*bus, *pool, address, 0);
            push(querySensorCapabilities);
        }
        if (queryCapabilities.isCommunications()) {
            log("[0x%d]: Communications module", address);
            addressIndex++;
            resume();
        }
        else {
            log("[0x%d]: Unknown module", address);
            addressIndex++;
            resume();
        }
    } else if (areSame(task, querySensorCapabilities)) {
        state->merge(address, querySensorCapabilities.replyMessage());
        auto sensor = (size_t)(querySensorCapabilities.sensor() + 1);
        if (sensor < queryCapabilities.numberOfSensors()) {
            querySensorCapabilities = QuerySensorCapabilities(*bus, *pool, address, sensor);
            push(querySensorCapabilities);
        } else {
            addressIndex++;
            resume();
        }

        retries = 0;
    } else {
        if (state->numberOfModules() == 0) {
            leds->noAttachedModules();
        } else {
            leds->haveAttachedModules();
        }
    }
}

void AttachedDevices::error(Task &task) {
    if (areSame(task, querySensorCapabilities)) {
        if (retries < NumberOfTwoWireRetries) {
            log("Retry %d/%d", retries, NumberOfTwoWireRetries);
            push(querySensorCapabilities);
            retries++;
        } else {
            state->scanFailure();
            retries = 0;
        }
    } else {
        addressIndex++;
        resume();
    }
}

}
