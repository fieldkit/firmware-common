#include <SPI.h>

#include "wifi.h"
#include "hardware.h"
#include "wifi_callbacks.h"

namespace fk {

constexpr const char LogName[] = "Wifi";

using Logger = SimpleLog<LogName>;

StaticWiFiCallbacks staticWiFiCallbacks;

Wifi::Wifi(WifiConnection &connection) : listen_(WifiServerPort, connection) {
}

bool Wifi::begin() {
    if (!initialized_) {
        WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

        WiFiSocketClass::callbacks = &staticWiFiCallbacks;

        if (WiFi.status() == WL_NO_SHIELD) {
            Logger::log("Error: No Wifi (%d, %d, %d, %d)", Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);
            available_ = false;
            return false;
        }

        available_ = true;

        auto fv = WiFi.firmwareVersion();
        Logger::log("Version: %s", fv);

        initialized_ = true;
    }

    disabled_ = false;

    return true;
}

void Wifi::disable() {
    listen_.end();
    WiFi.end();
    disabled_ = true;

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
    Logger::log("Disabled");
}

}

