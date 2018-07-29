#ifdef FK_CORE
#include <Base64.h>
#endif

#include "utils.h"
#include "wifi_states.h"
#include "wifi.h"
#include "app_servicer.h"

#include "simple_ntp.h"
#include "discovery.h"
#include "watchdog.h"

#include "live_data.h"
#include "gather_readings.h"
#include "idle.h"
#include "check_firmware.h"
#include "transmit_files.h"
#include "wifi_listening.h"
#include "wifi_disable.h"

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

class WifiCreateAp;
class WifiSyncTime;
class LiveDataReading;
class CheckFirmware;

void WifiState::serve() {
    services().state->updateIp(WiFi.localIP());

    // Before Scheduler so we service before transitioning to scheduled states.
    if (services().server->listen()) {
        transit(services().appServicer);
        return;
    }

    services().alive();
    services().discovery->task();
    services().scheduledTasks();

    if (services().liveData->takeReading()) {
        transit<LiveDataReading>();
    }
}

class WifiTryNetwork : public WifiState {
private:
    uint8_t index_;

public:
    WifiTryNetwork() {
    }

    WifiTryNetwork(uint8_t index) : index_(index) {
    }

public:
    const char *name() const override {
        return "WifiTryNetwork";
    }

public:
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

        auto localIp = WiFi.localIP();
        services().state->updateIp(localIp);

        IpAddress4 ip{ localIp };
        log("N[%d] Connected (%s) (%s)", index_, getWifiStatus(), ip.toString());
        transit<WifiSyncTime>();
    }
};

class WifiCreateAp : public WifiState {
public:
    const char *name() const override {
        return "WifiCreateAp";
    }

public:
    void task() override {
        // TODO: If done this before, skip?
        char name[32];
        getAccessPointName(name, sizeof(name));

        log("Creating AP '%s'... (%s)", name, getWifiStatus());
        IPAddress ip{ 192, 168, 2, 1 };
        auto status = WiFi.beginAP(name, 1, ip);
        if (status != WL_AP_LISTENING) {
            transit<WifiDisable>();
            services().state->updateIp(0);
            return;
        }

        services().state->updateIp(WiFi.localIP());

        transit_into<WifiListening>();
    }
};

class WifiSyncTime : public WifiState {
private:
    bool success_{ false };

public:
    const char *name() const override {
        return "WifiSyncTime";
    }

public:
    void task() override {
        if (!success_) {
            SimpleNTP ntp(clock);

            ntp.enqueued();

            while (elapsed() < NtpMaximumWait) {
                services().watchdog->task();
                if (!simple_task_run(ntp)) {
                    success_ = true;
                    break;
                }
            }
        }
        transit<CheckFirmware>();
    }

};

class LiveDataReading : public WifiState {
public:
    const char *name() const override {
        return "LiveDataReading";
    }

public:
    void task() override {
        GatherReadings gatherReadings{
            1,
            *services().state,
            *services().leds,
            *services().moduleCommunications
        };

        gatherReadings.enqueued();

        while (simple_task_run(gatherReadings)) {
            services().alive();
            services().moduleCommunications->task();
        }

        services().liveData->completed();

        back();
    }
};

class WifiCaptiveListening : public WifiState {
private:
    uint32_t began_{ 0 };

public:
    const char *name() const override {
        return "WifiCaptiveListening";
    }

public:
    void entry() override {
        WifiState::entry();
        began_ = fk_uptime();
    }

    void task() override {
        if (fk_uptime() - began_ > WifiCaptivitiyTimeout) {
            transit_into<WifiListening>();
            return;
        }

        serve();
    }

};

void WifiConnectionCompleted::task() {
    transit<WifiCaptiveListening>();
}

void WifiStartup::task() {
    if (services().wifi->disabled()) {
        if (!services().wifi->begin()) {
            error("Wifi initialize failed");
        }

        transit_into<WifiTryNetwork>((uint8_t)0);
    }
    else {
        if (services().wifi->hasInternetAccess()) {
            transit_into<WifiTransmitFiles>();
        }
        else {
            transit_into<WifiListening>();
        }
    }
}

}
