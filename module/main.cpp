/**
 * @file
 */
#include "example_module.h"

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    loginfof("Module", "Starting (%lu free)", fk_free_memory());

    fk::SensorInfo sensors[3] = {
      { "depth", "m" },
      { "temperature", "°C" },
      { "conductivity", "µS/cm" }
    };

    fk::SensorReading readings[3];

    fk::ModuleInfo info = {
        fk_module_ModuleType_SENSOR,
        8,
        3,
        1,
        "Dummy Module",
        "fk-dummy-module",
        sensors,
        readings
    };

    example::Sensors exampleSensors;
    example::ExampleModule module(info, exampleSensors);

    module.begin();

    while (true) {
        module.tick();
        exampleSensors.tick();
        delay(10);
    }
}

void loop() {
}
