#ifndef FK_WIFI_MESSAGE_BUFFER_H_INCLUDED
#define FK_WIFI_MESSAGE_BUFFER_H_INCLUDED

#include "message_buffer.h"

namespace fk {

class WifiMessageBuffer : public MessageBuffer {
private:
    WiFiClient *wcl;

public:
    void setConnection(WiFiClient &newClient) {
        wcl = &newClient;
    }

public:
    size_t read() override {
        debugfpln("Debug", "Read %p", wcl);
        auto pos = (size_t)wcl->read(ptr(), size());
        move(pos);
        return pos;
    }

    size_t write() override {
        if (position() == 0) {
            return 0;
        }
        auto bytesWritten = wcl->write(ptr(), position());
        if (bytesWritten > 0) {
            clear();
        }
        return bytesWritten;
    }

};

}

#endif
