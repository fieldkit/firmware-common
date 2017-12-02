#include <cstddef>
#include <cstdio>
#include <cstdint>

#include <Arduino.h>
#include <Wire.h>

#include <fk-module-protocol.h>
#include <fk-module.h>

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Module", "Starting (%d free)", fk_free_memory());

    fk::SensorInfo sensors[] = {
        {
            .sensor = 0,
            .name = "Depth",
            .unitOfMeasure = "m",
        },
        {
            .sensor = 1,
            .name = "Temperature",
            .unitOfMeasure = "°C",
        },
        {
            .sensor = 2,
            .name = "Conductivity",
            .unitOfMeasure = "µS/cm",
        }
    };

    fk::ModuleInfo info = {
        .address = 8,
        .numberOfSensors = 3,
        .name = "NOAA-CTD",
        .sensors = sensors,
    };

    fk::Module module(info);

    module.begin();

    while(true) {
        module.tick();
        delay(10);
    }
}

void loop() {
}

}
