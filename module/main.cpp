#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <Arduino.h>

#include <fk-module-protocol.h>
#include <fk-module.h>

class TakeFakeReadings : public fk::Task {
private:
    fk::SensorReading *readings;

public:
    TakeFakeReadings() : Task("TakeFakeReadings") {
    }

    TakeFakeReadings &into(fk::SensorReading *r) {
        readings = r;
        return *this;
    }

    fk::TaskEval task() override {
        for (size_t i = 0; i < 3; ++i) {
            readings[i].time = millis();
            readings[i].value = (float)i;
            readings[i].status = fk::SensorReadingStatus::Done;
        }

        return fk::TaskEval::done();
    }
};

class Sensors : public fk::ActiveObject {};

class ExampleModule : public fk::Module {
private:
    fk::Delay fiveSeconds{ 5000 };
    TakeFakeReadings takeFakeReadings;
    Sensors &sensors;

public:
    ExampleModule(fk::ModuleInfo &info, Sensors &sensors) : Module(info), sensors(sensors) {
    }

public:
    virtual void beginReading(fk::SensorReading *readings) override {
        debugfpln("Module", "Readings!");

        sensors.push(fiveSeconds);
        sensors.push(takeFakeReadings.into(readings));
    }

};

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Module", "Starting (%d free)", fk_free_memory());

    fk::ModuleInfo info = {
        8,
        3,
        "NOAA-CTD",
        { { "Depth", "m" },
          { "Temperature", "°C" },
          { "Conductivity", "µS/cm" }
        },
        { {}, {}, {} },
    };

    Sensors sensors;
    ExampleModule module(info, sensors);

    module.begin();

    while (true) {
        module.tick();
        sensors.tick();
        delay(10);
    }
}

void loop() {
}

}
