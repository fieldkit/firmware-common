#include "serial_number.h"

namespace fk {

SerialNumber::SerialNumber() {
    volatile uint32_t *ptr1 = (volatile uint32_t *)0x0080A00C;
    values[0] = *ptr1;
    volatile uint32_t *ptr = (volatile uint32_t *)0x0080A040;
    values[1] = *ptr;
    ptr++;
    values[2] = *ptr;
    ptr++;
    values[3] = *ptr;
}

const char *SerialNumber::toString() {
    sprintf(buffer, "%8lx%8lx%8lx%8lx", values[0], values[1], values[2], values[3]);
    return buffer;
}

}
