#ifndef FK_SERIAL_NUMBER_H_INCLUDED
#define FK_SERIAL_NUMBER_H_INCLUDED

#include <cinttypes>
#include <cstdio>

namespace fk {

class SerialNumber {
    static constexpr size_t MaximumSerialNumberLengthDwords = 4;
    static constexpr size_t MaximumSerialNumberLengthBytes = MaximumSerialNumberLengthDwords * 4;

private:
    uint32_t values[MaximumSerialNumberLengthDwords];
    char buffer[37];

public:
    SerialNumber();

public:
    const char *toString();
    uint32_t *toInts() {
        return values;
    }

};

}

#endif
