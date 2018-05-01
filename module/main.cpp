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

    fk::ModuleInfo info = {
        fk_module_ModuleType_SENSOR,
        8,
        3,
        "NOAA-CTD",
        { { "depth", "m" },
          { "temperature", "°C" },
          { "conductivity", "µS/cm" }
        },
        { { 0, 0, fk::SensorReadingStatus::Idle },
          { 0, 0, fk::SensorReadingStatus::Idle },
          { 0, 0, fk::SensorReadingStatus::Idle } },
    };

    example::Sensors sensors;
    example::ExampleModule module(info, sensors);

    module.begin();

    while (true) {
        module.tick();
        sensors.tick();
        delay(10);
    }
}

void loop() {
}
