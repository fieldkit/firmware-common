#ifndef FK_NETWORK_SETTINGS_H_INCLUDED
#define FK_NETWORK_SETTINGS_H_INCLUDED

namespace fk {

struct NetworkSettings {
    const char *ssid;
    const char *password;
    uint16_t port;
};

}

#endif
