#include <cstddef>
#include <cstdio>
#include <cstdint>

#include <Arduino.h>
#include <Wire.h>

#include <fk-module-protocol.h>

#include "debug.h"
#include "active_object.h"
#include "i2c.h"
#include "protobuf.h"
#include "module.h"

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Module", "Starting (%d free)", fk_free_memory());

    fk::Module module;

    module.begin(8);

    while(true) {
        module.tick();
        delay(10);
    }
}

void loop() {
}

}
