#include <Arduino.h>

#include "core_board.h"

namespace fk {

CoreBoard::CoreBoard(CoreBoardConfig config) : Board(config.config), config_(config) {
}

void CoreBoard::disable_everything() {
    Board::disable_everything();
    low(config_.gps_enable);
    low(config_.modules_enable);
    low(config_.wifi_enable);
}

void CoreBoard::enable_everything() {
    Board::enable_everything();
    high(config_.wifi_enable);
    high(config_.modules_enable);
    high(config_.gps_enable);
}

void CoreBoard::disable_gps() {
    low(config_.gps_enable);
}

void CoreBoard::enable_gps() {
    high(config_.gps_enable);
}

void CoreBoard::disable_wifi() {
    low(config_.wifi_enable);
}

void CoreBoard::enable_wifi() {
    high(config_.wifi_enable);
}

}
