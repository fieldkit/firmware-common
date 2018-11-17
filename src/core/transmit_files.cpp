#include "transmit_files.h"
#include "transmit_file.h"
#include "wifi_listening.h"

namespace fk {

class WifiTransmitFile : public WifiState {
private:
    FileCopySettings settings_{ FileNumber::Data };

public:
    WifiTransmitFile() {
    }

    WifiTransmitFile(FileCopySettings settings) : settings_(settings) {
    }

public:
    const char *name() const override {
        return "WifiTransmitFile";
    }

public:
    void task() override {
        TransmitFileTask task{
            *services().fileSystem,
            *services().state,
            *services().wifi,
            *services().httpConfig,
            settings_
        };

        // TODO: Maximum time in this state?
        while (true) {
            services().leds->task();
            services().watchdog->task();

            if (task.task().isDoneOrError()) {
                break;
            }
        }

        back();
    }
};

void WifiTransmitFiles::entry() {
    WifiState::entry();
    transmissions_[1] = {
        services().fileSystem->files().logFileNumber()
    };
}

void WifiTransmitFiles::task() {
    if (index_ == 2) {
        if (services().config.listening) {
            transit_into<WifiListening>();
        }
        else {
            transit<WifiDisable>();
        }
        index_ = 0;
    }
    else {
        transit_into<WifiTransmitFile>(transmissions_[index_]);
        index_++;
    }
}

}
