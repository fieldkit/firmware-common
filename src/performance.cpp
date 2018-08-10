#include <alogging/alogging.h>

#include "performance.h"

namespace fk {

Timer Performance::TwoWireSend;
Timer Performance::TwoWireReceive;
Timer Performance::Alive;
Timer Performance::Working;
Timer Performance::Copying;
Timer Performance::Idle;

constexpr const char LogName[] = "Perf";

using Logger = SimpleLog<LogName>;

void Performance::log() {
    Logger::trace("Alive(%lu) TWS(%lus) TWR(%lu) Working(%lu)", Alive.total(), TwoWireSend.total(), TwoWireReceive.total(), Working.total());
}

}
