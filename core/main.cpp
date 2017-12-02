#include <cstddef>
#include <cstdio>
#include <cstdint>

#include <Arduino.h>
#include <Wire.h>

#include "debug.h"
#include "pool.h"
#include "attached_devices.h"

namespace fk {

class ModulePoller : public ActiveObject {
private:
    Delay oneSecond;
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;

public:
    ModulePoller(uint8_t address, Pool *pool);

public:
    void done(Task &task) override;
    void error(Task &task) override;

};

ModulePoller::ModulePoller(uint8_t address, Pool *pool) : oneSecond(1000), beginTakeReading(pool, address), queryReadingStatus(pool, address) {
    push(oneSecond);
    push(beginTakeReading);
}

void ModulePoller::done(Task &task) {
    if (areSame(task, beginTakeReading)) {
        if (!beginTakeReading.isIdle()) {
            if (!beginTakeReading.isDone()) {
                push(oneSecond);
            }
            push(queryReadingStatus);
        }
        else {
            log("Have readings");
        }
    }

    if (areSame(task, queryReadingStatus)) {
        if (!queryReadingStatus.isIdle()) {
            if (!queryReadingStatus.isDone()) {
                push(oneSecond);
            }
            push(queryReadingStatus);
        }
        else {
            log("Have readings");
        }
    }
}

void ModulePoller::error(Task &task) {

}

}

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Core", "Starting (%d free)", fk_free_memory());

    Wire.begin();

    {
        fk::Pool pool("ROOT", 128);
        uint8_t addresses[] { 7, 8, 9, 0 };
        fk::AttachedDevices ad(addresses, 0, &pool);
        ad.scan();

        while (true) {
            ad.tick();

            if (ad.idle()) {
                break;
            }
        }
    }

    debugfpln("Core", "Idle");

    {
        fk::Pool pool("ROOT", 128);
        fk::ModulePoller poller(8, &pool);

        while (true) {
            poller.tick();

            if (poller.idle()) {
                break;
            }
        }
    }

    debugfpln("Core", "Idle");
}

void loop() {
}

}
