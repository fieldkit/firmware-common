#ifndef FK_WIFI_CAPTIVE_LISTENING_H_INCLUDED
#define FK_WIFI_CAPTIVE_LISTENING_H_INCLUDED

#include "wifi_states.h"
#include "wifi_client.h"

namespace fk {

class WifiDownloadFile : public WifiState {
private:
    FileCopySettings settings_{ FileNumber::Data };
    WifiConnection *connection_{ nullptr };

public:
    const char *name() const override {
        return "WifiDownloadFile";
    }

public:
    WifiDownloadFile() {
    }

    WifiDownloadFile(FileCopySettings settings, WifiConnection &connection) : settings_(settings), connection_(&connection) {
    }

public:
    void task() override;
};

}

#endif
