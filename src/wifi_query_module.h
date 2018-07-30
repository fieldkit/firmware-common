#ifndef FK_WIFI_QUERY_MODULE_H_INCLUDED
#define FK_WIFI_QUERY_MODULE_H_INCLUDED

#include "wifi_states.h"
#include "wifi_client.h"

namespace fk {

class WifiQueryModule : public WifiState {
private:
    WifiConnection *connection_{ nullptr };
    AppQueryMessage *query_{ nullptr };
    AppReplyMessage *reply_{ nullptr };

public:
    WifiQueryModule() {
    }

    WifiQueryModule(WifiConnection &connection, AppQueryMessage &query, AppReplyMessage &reply) : connection_(&connection), query_(&query), reply_(&reply) {
    }

public:
    const char *name() const override {
        return "WifiQueryModule";
    }

public:
    void task() override;

};

}

#endif
