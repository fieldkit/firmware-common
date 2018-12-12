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

template<typename T = MinimumFlashState>
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
    FlashState<T> flashState_{ flashFs_ };
    PendingReadings readings_{ *info_ };
    ModuleHardware hardware_;
    ModuleQueryMessage query_{ replyPool_ };
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
        &hardware_,
        &query_,
        nullptr,
    };

public:
    Module(TwoWireBus &bus, ModuleInfo &info) :
        bus_(&bus), info_(&info), twoWireChild_(bus, info.address) {
    }

public:
    virtual void begin() {
        moduleServices_.hooks = hooks();

        ModuleServicesState::services(moduleServices_);

        fsm_list::start();
    }

    virtual void tick() {
        ModuleState::current().task();
    }

    virtual ModuleHooks *hooks() {
        return nullptr;
    }

public:
    ModuleServices &moduleServices() {
        return moduleServices_;
    }

    FlashState<T> &flashState() {
        return flashState_;
    }

};

}

#endif
