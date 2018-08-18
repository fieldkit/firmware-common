/**
 * @file
 */
#include "example_module.h"

static void setup_serial() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);

        #ifndef FK_DEBUG_UART_REQUIRE_CONSOLE
        if (fk::fk_uptime() > 2000) {
            break;
        }
        #endif
    }

    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }
}

void setup() {
    setup_serial();

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
        "Testing Module",
        "fk-testing-module",
        sensors,
        readings
    };

    example::ExampleModule module(info);

    module.begin();

    while (true) {
        module.tick();
        delay(10);
    }
}

void loop() {
}
