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
public:
    void serve() {
        services().state->updateIp(WiFi.localIP());
        services().scheduler->task();
        services().discovery->task();
        services().server->task();
        if (services().server->isBusy()) {
            transit<WifiHandlingConnection>();
        }
    }
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

        // TODO: Maximum time in this state?
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
        // TODO: Right now scheduled tasks reset our elapsed time.
        if (elapsed() > 1000 * 60) {
            transit<WifiDisable>();
        }
        else {
            serve();
        }
    }

    void react(SchedulerEvent const &se) override {
        if (se.deferred) {
            warn("Scheduler Event!");
            transit(se.deferred);
        }
    }

};

class WifiDownloadFile : public WifiState {
private:
    FileCopySettings settings_{ FileNumber::StartupLog };

public:
    WifiDownloadFile() {
    }

    WifiDownloadFile(FileCopySettings settings) : settings_(settings) {
    }

public:
    void entry() override {
        WifiState::entry();
        log("WifiDownloadFile");
    }

    void task() override {
    }
};

class WifiModuleQuery : public WifiState {
public:
    WifiModuleQuery() {
    }

public:
    void entry() override {
        WifiState::entry();
        log("WifiModuleQuery");
    }

    void task() override {
        transit<WifiListening>();
    }
};

class WifiLiveData : public WifiState {
private:
    uint32_t interval_{ 0 };
    uint32_t lastReadings_{ 0 };
    uint32_t lastPolled_{ 0 };

public:
    WifiLiveData() {
    }

    WifiLiveData(uint32_t interval) : interval_(interval) {
    }

public:
    void react(LiveDataEvent const &lde) override {
        interval_ = lde.interval;
    }

    void react(AppQueryEvent const &aqe) override {
        if (aqe.type == fk_app_QueryType_QUERY_LIVE_DATA_POLL) {
            lastPolled_ = fk_uptime();
        }
    }

    void entry() override {
        WifiState::entry();
        log("WifiLiveData");

        lastReadings_ = 0;

        if (services().state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
            log("No attached modules.");
            transit<WifiListening>();
            return;
        }
    }

    void task() override {
        if (interval_ == 0) {
            log("Cancelled");
            back();
            return;
        }

        if (fk_uptime() - lastPolled_ > LivePollInactivity) {
            log("Stopped due to inactivity.");
            transit<WifiListening>();
            return;
        }

        if (fk_uptime() - lastReadings_ > interval_) {
            log("Readings");
            lastReadings_ = fk_uptime();
        }

        serve();
    }
};

class WifiHandlingConnection : public WifiState {
public:
    void react(LiveDataEvent const &lde) override {
        transit_into<WifiLiveData>(lde.interval);
    }

    void entry() override {
        WifiState::entry();
        log("WifiHandlingConnection");
        services().appServicer->enqueued();
    }

    void task() override {
        if (services().appServicer->task().isDoneOrError()) {
            // HACK: We can transition inside of the AppServicer.
            if (is_in_state<WifiHandlingConnection>()) {
                transit<WifiListening>();
            }
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
