#include "json_message_builder.h"

namespace fk {

JsonMessageBuilder::JsonMessageBuilder(CoreState &state, Clock &clock)
    : state(&state), clock(&clock) {
}

bool JsonMessageBuilder::isEmpty() {
    return state->numberOfReadings() == 0;
}

bool JsonMessageBuilder::write(Print &stream) {
    stream.print("{");
    writeLocation(stream);
    stream.print(",");
    writeTime(stream);
    stream.print(",");
    writeIdentity(stream);
    stream.print(",");
    writeValues(stream);
    stream.print("}");
    return true;
}

void JsonMessageBuilder::writeLocation(Print &stream) {
    auto location = state->getLocation();
    auto coordinates = location.coordinates;
    stream.print("\"location\":[");
    for (size_t i = 0; i < MaximumCoordinates; ++i) {
        if (i > 0) {
            stream.print(",");
        }
        stream.print(coordinates[i], 6);
    }
    stream.print("]");
}

void JsonMessageBuilder::writeTime(Print &stream) {
    stream.print("\"time\":");
    stream.print(clock->getTime());
}

void JsonMessageBuilder::writeIdentity(Print &stream) {
    auto identity = state->getIdentity();
    stream.print("\"device\":");
    stream.print("\"");
    stream.print(identity.device);
    stream.print("\"");
    stream.print(",");
    stream.print("\"stream\":");
    stream.print("\"");
    stream.print(identity.stream);
    stream.print("\"");
}

void JsonMessageBuilder::writeValues(Print &stream) {
    stream.print("\"values\":{");

    for (size_t sensor = 0; sensor < state->numberOfReadings(); ++sensor) {
        auto ar = state->getReading(sensor);
        if (sensor > 0) {
            stream.print(",");
        }
        stream.print("\"");
        stream.print(ar.sensor.name);
        stream.print("\":");
        stream.print("\"");
        stream.print(ar.reading.value);
        stream.print("\"");
    }

    stream.print("}");
}

}

