#ifndef FK_CORE_BOARD_H_INCLUDED
#define FK_CORE_BOARD_H_INCLUDED

#include "board.h"

namespace fk {

struct CoreBoardConfig {
    BoardConfig config;
    uint8_t spi_sd_cs;
    uint8_t spi_wifi_cs;
    uint8_t wifi_enable;
    uint8_t gps_enable;
    uint8_t modules_enable;
};

class CoreBoard : public Board {
private:
    CoreBoardConfig config_;

public:
    CoreBoard(CoreBoardConfig config);

public:
    void disable_everything() override;
    void enable_everything() override;
    void disable_gps();
    void enable_gps();
    void disable_wifi();
    void enable_wifi();

};

}

#endif
