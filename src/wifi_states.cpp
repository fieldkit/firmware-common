#include "wifi_states.h"
#include "wifi.h"

namespace fk {

class WifiListening;
class WifiCreateAp;
class WifiConnected;

class WifiState : public StateWithContext<MainServices> {
};

class WifiTryNetwork : public WifiState {
private:
    uint8_t index_;

public:
    WifiTryNetwork() {
    }

    WifiTryNetwork(uint8_t index) : index_(index) {
    }

public:
    void entry() override {
        log("WifiTryNetwork: %d", index_);
    }

    void task() override {
        if (index_ < 2) {
            transit_into<WifiTryNetwork>((uint8_t)(index_ + 1));
        }
        else {
            transit<WifiCreateAp>();
        }
    }
};

class WifiCreateAp : public WifiState {
public:
    void entry() override {
        log("WifiCreateAp");
    }

    void task() override {
        transit<WifiListening>();
    }
};

class WifiListening : public WifiState {
public:
    void entry() override {
        log("WifiListening");
    }

    void task() override {
        transit<Idle>();
    }
};

class WifiConnected : public WifiState {
public:
    void entry() override {
        log("WifiConnected");
    }

    void task() override {
    }
};

void WifiStartup::entry() {
    log("WifiStartup");
}

void WifiStartup::task() {
    transit_into<WifiTryNetwork>((uint8_t)0);
}

}
