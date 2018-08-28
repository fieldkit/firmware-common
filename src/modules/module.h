#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "message_buffer.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "module_fsm.h"
#include "watchdog.h"
#include "leds.h"
#include "two_wire.h"
#include "two_wire_child.h"
#include "pending_readings.h"
#include "hardware.h"

namespace fk {

class Module : public ModuleCallbacks {
private:
    TwoWireBus *bus_;
    ModuleInfo *info_;
    TwoWireChild twoWireChild_;
    ModuleStates states_;
    StaticPool<128> replyPool_{ "Reply" };
    Leds leds_;
    Watchdog watchdog_{ leds_ };
    SerialFlashFileSystem flashFs_{ watchdog_ };
    FlashState<MinimumFlashState> flashState_{ flashFs_ };
    PendingReadings readings_{ *info_ };
    ModuleHardware hardware_;
    ModuleServices moduleServices_{
        &replyPool_,
        info_,
        &leds_,
        &watchdog_,
        bus_,
        this,
        &twoWireChild_,
        &flashFs_,
        &flashState_,
        &readings_,
        &hardware_
    };

public:
    Module(TwoWireBus &bus, ModuleInfo &info, ModuleHardware hardware = { });

public:
    virtual void begin();
    virtual void tick();

public:
    ModuleServices &moduleServices() {
        return moduleServices_;
    }

};

}

#endif
