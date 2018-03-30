#include <SPI.h>

#include "wifi.h"
#include "utils.h"
#include "hardware.h"
#include "static_wifi_allocator.h"

namespace fk {

StaticWiFiAllocator staticWiFiAllocator;

Wifi::Wifi(CoreState &state, WifiConnection &connection, AppServicer &servicer, TaskQueue &taskQueue)
    : ActiveObject("Wifi"), state(&state), connection(&connection), connectToWifiAp(state), createWifiAp(state), listen(ServerPort, servicer, connection, taskQueue) {
}

void Wifi::begin() {
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

    WiFiSocketClass::allocator = &staticWiFiAllocator;

    if (WiFi.status() == WL_NO_SHIELD) {
        log("Error: no wifi (%d, %d, %d, %d)", Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);
        return;
    }
}

void Wifi::done(Task &task) {
    if (areSame(task, scanNetworks)) {
        push(createWifiAp);
    }
    lastActivityAt = millis();
}

void Wifi::error(Task &task) {
    if (areSame(task, connectToWifiAp)) {
        #ifdef FK_WIFI_AP_DISABLE
        disable();
        #else
        push(createWifiAp);
        #endif
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

void Wifi::ensureDisconnected() {
    if (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
        return;
    }

    WiFi.disconnect();

    while (!(WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS)) {
        ::delay(1000);
        log("Disconnecting(%s)...", getWifiStatus());
    }

    log("Disconnected(%s)", getWifiStatus());
}

void Wifi::disable() {
    listen.end();
    WiFi.end();
    lastActivityAt = millis();
    disabled = true;
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
    if (disabled) {
        if (!state->isBusy() && state->isReadingInProgress()) {
            if (millis() - lastActivityAt > WifiAwakenInterval) {
                log("Enabling...");
                begin();
                lastActivityAt = millis();
                disabled = false;
                version = 0;
            }
        }
        return;
    }

    auto newStatus = WiFi.status();
    if (newStatus != status) {
        log("Changed: %s", getWifiStatus());
        status = newStatus;
        lastActivityAt = millis();
        state->updateIp(WiFi.localIP());
        if (newStatus == WL_DISCONNECTED) {
            disable();
            return;
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
                    disable();
                }
            }
        }
        return;
    }

    log("New configuration...");
    version = settings.version;
    ensureDisconnected();
    listen.end();
    cancel();

    push(connectToWifiAp);
}

}
