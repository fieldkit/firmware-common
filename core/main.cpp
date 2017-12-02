#include <cstddef>
#include <cstdio>
#include <cstdint>

#include <Arduino.h>
#include <Wire.h>

#include "debug.h"
#include "pool.h"
#include "attached_devices.h"
#include "module_controller.h"

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
        fk::ModuleController controller(8, &pool);

        while (true) {
            controller.tick();

            if (controller.idle()) {
                break;
            }
        }
    }

    debugfpln("Core", "Idle");
}

void loop() {
}

}
