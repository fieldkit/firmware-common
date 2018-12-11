#ifndef FK_BOARD_H_INCLUDED
#define FK_BOARD_H_INCLUDED

#include <cinttypes>
#include <cstddef>

namespace fk {

template<size_t NCS, size_t NE>
struct BoardConfig {
    uint8_t spi_enable;
    uint8_t spi_flash_cs;
    uint8_t all_spi_cs[NCS];
    uint8_t all_enables[NE];
};

class SpiWrapper {
private:
    void *ptr_;

public:
    void begin();
    void end();

private:
    SpiWrapper(void *ptr) : ptr_(ptr) {
    }

public:
    static SpiWrapper spi();

};

class TwoWireWrapper {
private:
    void *ptr_;

public:
    void begin();
    void end();

private:
    TwoWireWrapper(void *ptr) : ptr_(ptr) {
    }

public:
    static TwoWireWrapper i2c1();
    static TwoWireWrapper i2c2();

};

class LowLevelBoard {
public:
    void low(uint8_t pin);
    void high(uint8_t pin);
    void disable_cs(uint8_t pin);
    void enable_cs(uint8_t pin);
    SpiWrapper spi();
    TwoWireWrapper i2c1();
    TwoWireWrapper i2c2();

};

template<class BC>
class Board : LowLevelBoard {
private:
    BC config_;

public:
    Board(BC config) : config_(config) {
    }

public:
    void disable_spi() {
        for (auto pin : config_.all_spi_cs) {
            disable_cs(pin);
        }
        low(config_.spi_enable);
        spi().end();
    }

    void enable_spi() {
        for (auto pin : config_.all_spi_cs) {
            enable_cs(pin);
        }
        high(config_.spi_enable);
        spi().begin();
    }

public:
    void disable_everything() {
        disable_spi();
        for (auto pin : config_.all_enables) {
            low(pin);
        }
        i2c1().end();
    }

    void enable_everything() {
        enable_spi();
        for (auto pin : config_.all_enables) {
            high(pin);
        }
        i2c1().begin();
    }
};

}

#endif
