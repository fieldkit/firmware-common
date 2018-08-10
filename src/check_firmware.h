#ifndef FK_CHECK_FIRMWARE_H_INCLUDED
#define FK_CHECK_FIRMWARE_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class CheckAllAttachedFirmware : public WifiState {
private:
    uint8_t index_;

public:
    CheckAllAttachedFirmware() : index_(0) {
    }

    CheckAllAttachedFirmware(uint8_t index) : index_(index) {
    }

public:
    const char *name() const override {
        return "CheckAllAttachedFirmware";
    }

public:
    void task() override;
};

class CheckFirmware : public WifiState {
private:
    const char *module_;
    const char *etag_;

public:
    CheckFirmware() : module_(nullptr) {
    }

    CheckFirmware(const char *module) : module_(module) {
    }

public:
    const char *name() const override {
        return "CheckFirmware";
    }

public:
    void task() override;

private:
    void check();


};

}

#endif
