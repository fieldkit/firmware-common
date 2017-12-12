#ifndef FK_NETWORK_SETTINGS_H_INCLUDED
#define FK_NETWORK_SETTINGS_H_INCLUDED

namespace fk {

constexpr uint8_t MaximumSsidLength = 33;
constexpr uint8_t MaximumPskLength = 33;
constexpr uint8_t MaximumRememberedNetworks = 2;

struct NetworkInfo {
    char ssid[MaximumSsidLength];
    char password[MaximumPskLength];

    NetworkInfo() {
        ssid[0] = 0;
        password[0] = 0;
    }

    NetworkInfo(const char *_ssid, const char *_password) {
        strncpy(ssid, _ssid, sizeof(ssid));
        strncpy(password, _password, sizeof(password));
    }
};

struct NetworkSettings {
    bool createAccessPoint;
    uint32_t version;
    NetworkInfo networks[MaximumRememberedNetworks];

    NetworkSettings() : createAccessPoint(true), version(0) {
        memzero(networks, sizeof(networks));
    }

    NetworkSettings(bool ap, NetworkInfo info) : createAccessPoint(ap), version(0) {
        memzero(networks, sizeof(networks));
        memcpy(&networks[0], &info, sizeof(NetworkInfo));
    }
};

}

#endif
