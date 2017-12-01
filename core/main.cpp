#include <cstddef>
#include <cstdio>
#include <cstdint>

#include <Arduino.h>
#include <Wire.h>

#include "debug.h"
#include "pool.h"
#include "attached_devices.h"

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Core", "Starting (%d free)", fk_free_memory());

    Wire.begin();

    uint8_t addresses[] { 7, 8, 9, 0 };
    fk::Pool pool("ROOT", 128);
    fk::AttachedDevices ad(addresses, 0, &pool);
    ad.scan();

    while(true) {
        ad.tick();

        if (ad.idle()) {
            break;
        }
    }

    debugfpln("Core", "Idle");

    while (true) {
    }
}

void loop() {
}

}
