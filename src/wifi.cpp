#include <SPI.h>

#include "wifi.h"
#include "utils.h"
#include "hardware.h"
#include "static_wifi_allocator.h"

namespace fk {

StaticWiFiAllocator staticWiFiAllocator;

Wifi::Wifi(CoreState &state, WifiConnection &connection, AppServicer &servicer, TaskQueue &taskQueue)
    : ActiveObject("Wifi"), state(&state), connection(&connection), connectToWifiAp(state), createWifiAp(state), listen(WifiServerPort, servicer, connection, taskQueue) {
}

void Wifi::begin() {
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

    WiFiSocketClass::allocator = &staticWiFiAllocator;

    if (WiFi.status() == WL_NO_SHIELD) {
        log("Error: no wifi (%d, %d, %d, %d)", Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);
        return;
    }

    auto fv = WiFi.firmwareVersion();
    log("Version: %s", fv);

    lastActivityAt = millis();
    disabled = false;
    lastStatusAt = 0;
    version = 0;
}

void Wifi::done(Task &task) {
    if (areSame(task, scanNetworks)) {
        push(createWifiAp);
    }
    lastActivityAt = millis();
}

void Wifi::error(Task &task) {
    if (areSame(task, connectToWifiAp)) {
        if (triedAp) {
            disable();
        } else {
            #ifndef FK_WIFI_ALWAYS_ON
            triedAp = true;
            #endif
            #ifdef FK_WIFI_AP_DISABLE
            disable();
            #else
            push(createWifiAp);
            #endif
        }
    }
    else if (areSame(task, createWifiAp)) {
        disable();
    }
    else {
        push(delay);
    }
    lastActivityAt = millis();
}

bool Wifi::readyToServe() {
    return status == WL_CONNECTED || status == WL_AP_CONNECTED;
}

bool Wifi::isListening() {
    return status == WL_AP_LISTENING;
}

void Wifi::traceStatus() {
    IpAddress4 ip{ WiFi.localIP() };
    auto rssi = WiFi.RSSI();
    trace("Status %s/%s readyToServe=%d isListening=%d busy=%d (%s) (rssi = %ld)", getWifiStatus(WiFi.status()), getWifiStatus(status), readyToServe(), isListening(), busy, ip.toString(), rssi);
}

void Wifi::disable() {
    listen.end();
    WiFi.end();
    lastActivityAt = millis();
    disabled = true;
    status = WL_IDLE_STATUS;
    state->updateIp(0);

    // Allow me to explain:
    // I was seeing this very strange problem where after a Disable
    // the WDT would kick off. It was always preceeded by fkfs
    // activity and so things eventually led me to the SPI bus code.
    // I was inside of sd_raw, in the code for sd_raw_command.
    // There's a call to sd_raw_flush and that's where the hang
    // occured. In there it waits until the "Data Register is Empty"
    // (ATSAMD SerCOM.CPP line 305) I'm assuming there's some kind
    // of flush that doesn't happen. This "reset" fixes the problem.
    // If I do this, and don't do the call to maxLowPowerMode
    // anymore and also move the log call you see below to after the
    // reset of the bus things work more often. There is still a WDT
    // reset occuring, though just doesn't seem to happen around here.
    SPI.end();
    SPI.begin();

    log("Disabled");
}

void Wifi::idle() {
    if (!state->isReady()) {
        return;
    }

    if (disabled) {
        if (millis() - lastActivityAt > WifiAwakenInterval) {
            if (!state->isBusy() && !state->isReadingInProgress()) {
                log("Enabling...");
                begin();
            }
        }
        return;
    }

    if (millis() - lastStatusAt > WifiStatusInterval) {
        traceStatus();
        lastStatusAt = millis();
    }

    auto newStatus = WiFi.status();
    if (newStatus != status) {
        log("Changed: %s", getWifiStatus());
        status = newStatus;
        lastActivityAt = millis();
        state->updateIp(WiFi.localIP());
        switch (newStatus) {
        case WL_AP_CONNECTED: {
            traceStatus();
            break;
        }
        case WL_DISCONNECTED: {
            disable();
            return;
        }
        }
    }

    if (status == WL_NO_SHIELD) {
        return;
    }

    auto settings = state->getNetworkSettings();
    if (version == settings.version) {
        if (readyToServe()) {
            service(listen);
        }
        else {
            listen.end();
        }

        state->setBusy(connection->isConnected());

        if (!busy && listen.inactive()) {
            if (millis() - lastActivityAt > WifiInactivityTimeout) {
                if (isListening() || readyToServe())  {
                    #ifndef FK_WIFI_ALWAYS_ON
                    disable();
                    #endif
                }
            }
        }
        return;
    }

    log("New configuration (%lu vs %lu)", version, settings.version);
    disable();
    ::delay(1000);
    begin();
    cancel();

    push(connectToWifiAp);

    // Afterwards cause the above clears the state.
    version = settings.version;
}

}
