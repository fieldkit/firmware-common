#include "attached_devices.h"
#include "leds.h"
#include "tuning.h"

namespace fk {

AttachedDevices::AttachedDevices(TwoWireBus &bus, uint8_t *addresses, CoreState &state, Leds &leds, ModuleCommunications &communications, Pool &pool)
    : Task("AttachedDevices"), bus(&bus), addresses(addresses), addressIndex{ 0 }, state(&state), leds(&leds), protocol(communications, pool) {
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
            query(address);
            break;
        } else {
            log("Done scanning.");
            state->doneScanning();
            peripherals.twoWire1().release(this);
            break;
        }

        addressIndex++;
    }
}

TaskEval AttachedDevices::task() {
    if (state->isBusy()) {
        return TaskEval::idle();
    }

    auto rescanInterval = (state->numberOfModules() == 0 ? RescanExistingModulesInterval : RescanExistingModulesInterval);
    if (lastScanAt == 0 || millis() - lastScanAt > rescanInterval) {
        log("Starting scan...");
        lastScanAt = millis();
        scan();
    }

    auto finished = protocol.handle();
    if (finished) {
        if (finished.error()) {
            error(finished);
        }
        else {
            done(finished);
        }
    }

    return TaskEval::idle();
}

void AttachedDevices::query(uint8_t address) {
    log("[0x%d]: Query", address);

    queryCapabilities = QueryCapabilities();
    protocol.push(address, queryCapabilities);
}

void AttachedDevices::done(ModuleProtocolHandler::Finished &finished) {
    auto address = addresses[addressIndex];
    if (finished.is(queryCapabilities)) {
        state->merge(address, *finished.reply);
        if (queryCapabilities.isSensor()) {
            log("[0x%d]: Sensor module", address);
            querySensorCapabilities = QuerySensorCapabilities();
            protocol.push(address, querySensorCapabilities);
        }
        else if (queryCapabilities.isCommunications()) {
            log("[0x%d]: Communications module", address);
            addressIndex++;
            resume();
        }
        else {
            log("[0x%d]: Unknown module", address);
            addressIndex++;
            resume();
        }
    } else if (finished.is(querySensorCapabilities)) {
        state->merge(address, *finished.reply);
        if (querySensorCapabilities.getSensor() < queryCapabilities.getNumberOfSensors()) {
            protocol.push(address, querySensorCapabilities);
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

void AttachedDevices::error(ModuleProtocolHandler::Finished &finished) {
    if (finished.is(querySensorCapabilities)) {
        if (retries < NumberOfTwoWireRetries) {
            log("Retry %d/%d", retries, NumberOfTwoWireRetries);
            protocol.push(addresses[addressIndex], querySensorCapabilities);
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
