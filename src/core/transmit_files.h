#ifndef FK_TRANSMIT_FILES_H_INCLUDED
#define FK_TRANSMIT_FILES_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiTransmitFiles :  public WifiState {
private:
    bool first_{ true };
    uint8_t index_{ 0 };
    FileCopySettings transmissions_[2] = {
        { FileNumber::Data },
        { FileNumber::LogsA }
    };

public:
    const char *name() const override {
        return "WifiTransmitFiles";
    }

public:
    void entry();
    void task();
};

}

#endif
