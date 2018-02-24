#ifndef FK_WIFI_CLIENT_H_INCLUDED
#define FK_WIFI_CLIENT_H_INCLUDED

namespace fk {

class WifiClient {
private:

public:

};

};

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

#endif
