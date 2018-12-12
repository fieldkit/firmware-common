#ifndef FK_BOARD_H_INCLUDED
#define FK_BOARD_H_INCLUDED

#include <cinttypes>
#include <cstddef>

namespace fk {

struct BoardConfig {
    uint8_t spi_enable;
    uint8_t spi_flash_cs;
    uint8_t all_spi_cs[4];
    uint8_t all_enables[4];
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
    SpiWrapper spi();
    TwoWireWrapper i2c1();
    TwoWireWrapper i2c2();

public:
    virtual void enable_everything() { }
    virtual void disable_everything() { }
    virtual void enable_spi() { }
    virtual void disable_spi() { }

protected:
    void low(uint8_t pin);
    void high(uint8_t pin);
    void disable_cs(uint8_t pin);
    void enable_cs(uint8_t pin);

};

class Board : LowLevelBoard {
private:
    BoardConfig config_;

public:
    Board(BoardConfig config) : config_(config) {
    }

public:
    uint8_t flash_cs() const {
        return config_.spi_flash_cs;
    }

    void disable_spi() override {
        for (auto pin : config_.all_spi_cs) {
            if (pin == 0) {
                break;
            }
            disable_cs(pin);
        }
        low(config_.spi_enable);
        spi().end();
    }

    void enable_spi() override {
        for (auto pin : config_.all_spi_cs) {
            if (pin == 0) {
                break;
            }
            enable_cs(pin);
        }
        high(config_.spi_enable);
        spi().begin();
    }

public:
    void disable_everything() override {
        disable_spi();
        for (auto pin : config_.all_enables) {
            if (pin == 0) {
                break;
            }
            low(pin);
        }
        i2c1().end();
    }

    void enable_everything() override {
        enable_spi();
        for (auto pin : config_.all_enables) {
            if (pin == 0) {
                break;
            }
            high(pin);
        }
        i2c1().begin();
    }
};

}

#endif
