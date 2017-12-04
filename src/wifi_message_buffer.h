#ifndef FK_WIFI_MESSAGE_BUFFER_H_INCLUDED
#define FK_WIFI_MESSAGE_BUFFER_H_INCLUDED

#include "message_buffer.h"

namespace fk {

class WifiMessageBuffer : public MessageBuffer {
public:
    size_t read(WiFiClient &wcl) {
        auto pos = (size_t)wcl.read(ptr(), size());
        move(pos);
        return pos;
    }
};

}

#endif
