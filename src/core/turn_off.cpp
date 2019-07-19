#include "turn_off.h"
#include "file_system.h"
#include "configuration.h"
#include "wifi.h"
#include "user_button.h"
#include "core_board.h"
#include "reboot_device.h"

namespace fk {

extern CoreBoard board;

void TurnOff::task() {
    log("Off!");

    services().wifi->disable();
    services().state->updateIp(0);
    services().fileSystem->flush();
    services().leds->off();

    board.disable_everything();

    while (true) {
        auto delayed = false;

        if (configuration.sleeping.deep) {
            if (!fk_console_attached()) {
                services().watchdog->sleep(SleepMaximumGranularity);
                delayed = true;
            }
        }

        if (!delayed) {
            delay(1000);
        }

        services().watchdog->idling();

        services().watchdog->task();
        services().button->task();
        services().leds->task();

        if (transitioned()) {
            break;
        }
    }
}

void TurnOff::react(UserButtonEvent const &ignored) {
    services().leds->notifyButtonLong();
    log("UBE");
}

void TurnOff::react(MinorButtonPressEvent const &ignored) {
    services().leds->off();
    transit<RebootDevice>();
    log("MBPE");
}

void TurnOff::react(ShortButtonPressEvent const &ignored) {
    services().leds->off();
    transit<RebootDevice>();
    log("SBPE");
}

void TurnOff::react(LongButtonPressEvent const &ignored) {
    services().leds->off();
    transit<RebootDevice>();
    log("LBPE");
}

}
