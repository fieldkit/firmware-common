#ifndef FK_BUFFERS_H_INCLUDED
#define FK_BUFFERS_H_INCLUDED

#include <cstdint>

namespace fk {

class Buffers {
private:
    uint8_t outgoing[256];
    uint8_t incoming[256];

};

}

#endif
