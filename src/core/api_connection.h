#ifndef FK_API_CONNECTION_H_INCLUDED
#define FK_API_CONNECTION_H_INCLUDED

#include "wifi_states.h"
#include "pool.h"
#include "app_messages.h"
#include "wifi_client.h"
#include "message_buffer.h"

namespace fk {

class ApiConnection : public WifiState {
protected:
    AppQueryMessage query_;
    AppReplyMessage reply_;

private:
    WifiConnection *connection_;
    Pool *pool_;
    uint32_t dieAt_{ 0 };
    size_t bytesRead_{ 0 };

public:
    ApiConnection(WifiConnection &connection, Pool &pool);

public:
    const char *name() const override {
        return "ApiConnection";
    }

public:
    void entry() override;
    void task() override;
    bool flushAndClose();

protected:
    WifiConnection &connection() {
        return *connection_;
    }

    MessageBuffer &buffer() {
        return connection_->getBuffer();
    }

private:
    bool service();
    bool readQuery();

public:
    virtual bool handle();

};

}

#endif
