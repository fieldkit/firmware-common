#include "radio_service.h"

namespace fk {

#ifdef FK_ENABLE_RADIO

RadioService::RadioService() : Task("Radio"), radio{ 5, 2, 0, 3 } {
}

bool RadioService::setup() {
    return radio.setup();
}

TaskEval RadioService::task() {
    return TaskEval::busy();
}

#endif

}
