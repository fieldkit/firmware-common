#include <Arduino.h>
#include <wiring_private.h>

#include "hardware.h"

namespace fk {

#ifdef FK_HARDWARE_SERIAL2_ENABLE

Uart Serial2(&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);

#endif

void SerialPort::begin(int32_t baud) {
    uart->begin(baud);

#ifdef FK_HARDWARE_SERIAL2_ENABLE
    if (*uart == Serial2) {
        pinPeripheral(10, PIO_SERCOM);
        pinPeripheral(11, PIO_SERCOM);
    }
#endif
}

bool SerialPort::available() {
    return uart->available();
}

int32_t SerialPort::read() {
    return uart->read();
}

void SerialPort::println(const char *str) {
    uart->println(str);
}

}

#ifdef FK_HARDWARE_SERIAL2_ENABLE

extern "C" {

void SERCOM1_Handler() {
    fk::Serial2.IrqHandler();
}

}

#endif
