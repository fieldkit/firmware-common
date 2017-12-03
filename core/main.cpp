#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <Arduino.h>

#include "app_servicer.h"
#include "attached_devices.h"
#include "debug.h"
#include "i2c.h"
#include "module_controller.h"
#include "pool.h"
#include "wifi.h"

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Core", "Starting (%d free)", fk_free_memory());

    fk::i2c_begin();

    {
        uint8_t addresses[]{ 7, 8, 9, 0 };
        fk::Pool pool("ROOT", 128);
        fk::AttachedDevices ad(addresses, 0, pool);
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
        fk::ModuleController modules(8, pool);
        fk::NetworkSettings networkSettings {
            .ssid = "Cottonwood",
            .password = "asdfasdf",
            .port = 54321,
        };
        fk::Wifi wifi(networkSettings, modules);

        // TODO: Fix that this is blocking when connecting.
        wifi.begin();

        modules.beginReading();

        while (true) {
            modules.tick();
            wifi.tick();
        }
    }
}

void loop() {
}

}
