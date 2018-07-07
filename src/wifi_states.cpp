#ifdef FK_CORE
#include <Base64.h>
#endif

#include "wifi_states.h"
#include "wifi.h"
#include "utils.h"

#include "simple_ntp.h"
#include "discovery.h"
#include "watchdog.h"

namespace fk {

#ifdef FK_CORE
static void getAccessPointName(char *name, size_t size) {
    auto length = base64_enc_len(deviceId.length());
    char unique[length + 3];
    unique[0] = 'F';
    unique[1] = 'K';
    unique[2] = '-';
    base64_encode(unique + 3, (char *)deviceId.toBuffer(), deviceId.length());
    unique[length + 3 - 1] = 0; // Trim '='
    strncpy(name, unique, size);
}
#else
static void getAccessPointName(char *name, size_t size) {
    strncpy(name, "FK-UNKONWN", size);
}
#endif

class WifiDisable;
class WifiListening;
class WifiCreateAp;
class WifiSyncTime;
class WifiTransmit;
class WifiHandlingConnection;

class WifiState : public WifiServicesState {
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
        WifiState::entry();
        log("WifiTryNetwork: %d", index_);
    }

    void task() override {
        if (index_ >= MaximumRememberedNetworks) {
            log("No more networks (%s)", getWifiStatus());
            transit<WifiCreateAp>();
            return;
        }

        auto settings = services().state->getNetworkSettings();
        auto network = settings.networks[index_];
        if (network.ssid[0] == 0) {
            log("N[%d] No network configured (%s)", index_, getWifiStatus());
            transit_into<WifiTryNetwork>((uint8_t)(index_ + 1));
            return;
        }

        log("N[%d] Connecting to AP '%s'... (%s)", index_, network.ssid, getWifiStatus());
        if (WiFi.begin(network.ssid, network.password) != WL_CONNECTED) {
            log("N[%d] Failed (%s)", index_, getWifiStatus());
            transit_into<WifiTryNetwork>((uint8_t)(index_ + 1));
            return;
        }

        IpAddress4 ip{ WiFi.localIP() };
        log("N[%d] Connected (%s) (%s)", index_, getWifiStatus(), ip.toString());
        transit<WifiSyncTime>();
    }
};

class WifiCreateAp : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiCreateAp");
    }

    void task() override {
        // TODO: If done this before, skip.
        char name[32];
        getAccessPointName(name, sizeof(name));

        log("Creating AP '%s'... (%s)", name, getWifiStatus());
        IPAddress ip{ 192, 168, 2, 1 };
        auto status = WiFi.beginAP(name, 1, ip);
        if (status != WL_AP_LISTENING) {
            transit<WifiDisable>();
        }

        transit<WifiListening>();
    }
};

class WifiSyncTime : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiSyncTime");
    }

    void task() override {
        SimpleNTP ntp(clock, *services().wifi);

        services().state->updateIp(WiFi.localIP());

        ntp.enqueued();

        while (elapsed() < NtpMaximumWait) {
            services().watchdog->task();
            auto e = ntp.task();
            if (e.isDoneOrError()) {
                break;
            }
        }

        transit<WifiTransmit>();
    }

};

class WifiTransmit :  public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiTransmit");
    }

    void task() override {
        transit<WifiListening>();
    }
};

class WifiListening : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiListening");
    }

    void task() override {
        services().state->updateIp(WiFi.localIP());

        services().discovery->task();

        services().server->task();

        if (services().server->isBusy()) {
            transit<WifiHandlingConnection>();
        }

        if (elapsed() > 1000 * 60) {
            transit<WifiDisable>();
        }
    }
};

class WifiHandlingConnection : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiHandlingConnection");
    }

    void task() override {
        services().server->task();

        if (!services().server->isBusy()) {
            transit<WifiListening>();
        }
    }
};

class WifiDisable : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiDisable");
        services().wifi->disable();
    }

};

void WifiStartup::entry() {
    WifiServicesState::entry();
    log("WifiStartup");
}

void WifiStartup::task() {
    if (!initialized_) {
        if (!services().wifi->begin()) {
            error("Wifi initialize failed");
            // TODO: transit_into<Panic>();
        }
        initialized_ = true;
    }

    transit_into<WifiTryNetwork>((uint8_t)0);
}

}
