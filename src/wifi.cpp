#include <SPI.h>

#include "wifi.h"
#include "utils.h"
#include "hardware.h"
#include "static_wifi_allocator.h"

namespace fk {

StaticWiFiAllocator staticWiFiAllocator;

Wifi::Wifi(WifiConnection &connection, AppServicer &servicer)
    : Task("Wifi"), connection(&connection), listen(WifiServerPort, servicer, connection) {
}

bool Wifi::begin() {
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

    WiFiSocketClass::allocator = &staticWiFiAllocator;

    if (WiFi.status() == WL_NO_SHIELD) {
        log("Error: no wifi (%d, %d, %d, %d)", Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ,
            Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);
        return false;
    }

    auto fv = WiFi.firmwareVersion();
    log("Version: %s", fv);

    disabled = false;

    return true;
}

void Wifi::disable() {
    listen.end();
    WiFi.end();
    disabled = true;

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

}
