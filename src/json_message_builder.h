#ifndef FK_JSON_MESSAGE_BUILDER_H_INCLUDED
#define FK_JSON_MESSAGE_BUILDER_H_INCLUDED

#include "transmissions.h"
#include "rtc.h"

namespace fk {

constexpr char FkMessageJsonContentType[] = "application/vnd.fk.message+json";

class JsonMessageBuilder : public MessageBuilder {
private:
    CoreState *state;
    Clock *clock;

public:
    JsonMessageBuilder(CoreState &state, Clock &clock) : state(&state), clock(&clock) {
    }

public:
    bool write(Print &stream) override;

public:
    const char *getContentType() override {
        return FkMessageJsonContentType;
    }

private:
    void writeLocation(Print &stream);
    void writeTime(Print &stream);
    void writeIdentity(Print &stream);
    void writeValues(Print &stream);

};

}

#endif
