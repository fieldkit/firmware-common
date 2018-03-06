#include "hardware.h"

namespace fk {

#ifdef FK_NATURALIST

Uart Serial2(&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);

Uart &Hardware::gpsUart = Serial2;

#else

Uart &Hardware::gpsUart = Serial1;

#endif

}

#ifdef FK_NATURALIST

extern "C" {

void SERCOM1_Handler() {
    fk::Serial2.IrqHandler();
}

}

#endif
