#include <cstddef>
#include <cstdio>
#include <cstdint>

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
            readings[i].sensor = i;
            readings[i].time = millis();
            readings[i].value = (float)i;
            readings[i].status = fk::SensorReadingStatus::Done;
        }

        return fk::TaskEval::done();
    }
};

class Sensors : public fk::ActiveObject {
};

class ExampleModule : public fk::Module {
private:
    fk::Delay fiveSeconds { 5000 };
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

    fk::SensorReading readings[] = {
        {},
        {},
        {},
    };

    fk::ModuleInfo info = {
        .address = 8,
        .numberOfSensors = 3,
        .name = "NOAA-CTD",
        .sensors = sensors,
        .readings = readings,
    };

    Sensors sensorsQueue;
    ExampleModule module(info, sensorsQueue);

    module.begin();

    while(true) {
        module.tick();
        sensorsQueue.tick();
        delay(10);
    }
}

void loop() {
}

}
