#ifdef FK_CORE
#include <Base64.h>
#endif

#include "wifi_states.h"
#include "wifi.h"
#include "utils.h"

#include "simple_ntp.h"
#include "discovery.h"
#include "watchdog.h"

#include "transmit_file.h"

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
class WifiTransmitFiles;
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
private:
    bool success_{ false };

public:
    void entry() override {
        WifiState::entry();
        log("WifiSyncTime");
    }

    void task() override {
        if (!success_) {
            SimpleNTP ntp(clock, *services().wifi);

            services().state->updateIp(WiFi.localIP());

            ntp.enqueued();

            while (elapsed() < NtpMaximumWait) {
                services().watchdog->task();
                auto e = ntp.task();
                if (e.isDone()) {
                    success_ = true;
                }
                if (e.isDoneOrError()) {
                    break;
                }
            }
        }
        transit<WifiTransmitFiles>();
    }

};

class WifiTransmitFile : public WifiState {
private:
    FileCopySettings settings_{ FileNumber::StartupLog };

public:
    WifiTransmitFile() {
    }

    WifiTransmitFile(FileCopySettings settings) : settings_(settings) {
    }

public:
    void entry() override {
        WifiState::entry();
        log("WifiTransmitFile");
    }

    void task() override {
        TransmitFileTask task{
            *services().fileSystem,
            *services().state,
            *services().wifi,
            *services().httpConfig,
            settings_
        };

        while (true) {
            services().leds->task();
            services().watchdog->task();

            if (task.task().isDoneOrError()) {
                break;
            }
        }

        back();
    }
};

class WifiTransmitFiles :  public WifiState {
private:
    size_t index_{ 0 };
    FileCopySettings transmissions_[2] = {
        { FileNumber::StartupLog },
        { FileNumber::Data }
    };

public:
    WifiTransmitFiles() {
    }

public:
    void entry() override {
        WifiState::entry();
        log("WifiTransmitFiles");
    }

    void task() override {
        if (index_ == 2) {
            transit<WifiListening>();
        }
        else {
            transit_into<WifiTransmitFile>(transmissions_[index_]);
            index_++;
        }
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

        services().scheduler->task();

        services().discovery->task();

        services().server->task();

        if (services().server->isBusy()) {
            transit<WifiHandlingConnection>();
        }

        // TODO: Right now scheduled tasks reset our elapsed time.
        if (elapsed() > 1000 * 60) {
            transit<WifiDisable>();
        }
    }

    void react(SchedulerEvent const &se) override {
        if (se.deferred) {
            warn("Scheduler Event!");
            transit(se.deferred);
        }
    }

};

class WifiHandlingConnection : public WifiState {
public:
    void entry() override {
        WifiState::entry();
        log("WifiHandlingConnection");
        services().appServicer->enqueued();
    }

    void task() override {
        if (services().appServicer->task().isDoneOrError()) {
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
        services().state->updateIp(0);
        transit<Idle>();
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
