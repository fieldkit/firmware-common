#include "factory_reset_check.h"
#include "factory_reset.h"
#include "user_button.h"
#include "startup.h"

namespace fk {

void FactoryResetCheck::task() {
    while (services().button->pressed() || services().button->pending()) {
        services().alive();

        if (transitioned()) {
            return;
        }
    }

    transit<StartSystem>();
}

void FactoryResetCheck::react(ShortButtonPressEvent const &sbpe) {
}

void FactoryResetCheck::react(LongButtonPressEvent const &lbpe) {
    transit<FactoryReset>();
}

}
