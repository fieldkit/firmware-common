#ifndef FK_SERIAL_PORT_H_INCLUDED
#define FK_SERIAL_PORT_H_INCLUDED

namespace fk {

#ifdef FK_HARDWARE_SERIAL2_ENABLE
extern Uart Serial2;
#endif

class SerialPort {
private:
    Uart *uart;

public:
    SerialPort(Uart &uart) : uart(&uart) {
    }

public:
    void begin(int32_t baud);
    bool available();
    int32_t read();
    void println(const char *str);

};

}

#endif
