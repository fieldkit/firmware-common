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
            readings[i].value = random(10, 20);
            readings[i].status = fk::SensorReadingStatus::Done;
        }

        return fk::TaskEval::done();
    }
};

class Sensors : public fk::ActiveObject {
public:
    Sensors() : fk::ActiveObject("Sensors") {
    }
};

class ExampleModule : public fk::Module {
private:
    fk::Delay fiveSeconds{ 5000 };
    TakeFakeReadings takeFakeReadings;
    Sensors &sensors;

public:
    ExampleModule(fk::ModuleInfo &info, Sensors &sensors) :
        Module(info), sensors(sensors) {
    }

public:
    virtual void beginReading(fk::SensorReading *readings) override {
        log("Readings!");

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
        { { 0, 0, fk::SensorReadingStatus::Idle },
          { 0, 0, fk::SensorReadingStatus::Idle },
          { 0, 0, fk::SensorReadingStatus::Idle } },
    };

    fk::Clock clock;
    fk::Watchdog watchdog;

    clock.begin();

    Sensors sensors;
    ExampleModule module(info, sensors);

    module.begin();

    while (true) {
        module.tick();
        sensors.tick();
        watchdog.tick();
        delay(10);
    }
}

void loop() {
}

}
