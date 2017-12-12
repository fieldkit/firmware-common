#ifndef FK_NETWORK_SETTINGS_H_INCLUDED
#define FK_NETWORK_SETTINGS_H_INCLUDED

namespace fk {

struct NetworkSettings {
    bool createAccessPoint;
    const char *ssid;
    const char *password;
};

}

#endif
