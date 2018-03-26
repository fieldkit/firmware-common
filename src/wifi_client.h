#ifndef FK_WIFI_CLIENT_H_INCLUDED
#define FK_WIFI_CLIENT_H_INCLUDED

#include "wifi_message_buffer.h"

namespace fk {

template<typename T>
class Resource {
private:
    uint32_t acquiredAt{ 0 };
    T target;

public:
    bool available() {
        return acquiredAt == 0;
    }

    T &resource() {
        return target;
    }

    T &acquire() {
        fk_assert(available());
        acquiredAt = millis();
        return resource();
    }

    bool tryAcquire() {
        if (available()) {
            acquire();
            return true;
        }
        return false;
    }

    void release() {
        fk_assert(!available());
        acquiredAt = 0;
    }
};

class WifiConnection {
private:
    WiFiClient wcl;
    WifiMessageBuffer buffer;

public:
    void setConnection(WiFiClient &newClient) {
        wcl = newClient;
        buffer.setConnection(wcl);
    }

    bool isOpen() {
        return wcl && wcl.connected();
    }

    void close() {
        wcl.flush();
        wcl.stop();
        wcl = WiFiClient{};
    }

    bool available() {
        return wcl.available();
    }

    WifiMessageBuffer &getBuffer() {
        return buffer;
    }

    void flush() {
        if (!buffer.empty()) {
            buffer.write();
        }
    }

    size_t read() {
        if (wcl.available()) {
            return buffer.read();
        }
        return 0;
    }

};

}

#endif
