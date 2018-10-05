#ifndef FK_MODULE_FSM_H_INCLUDED
#define FK_MODULE_FSM_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "pool.h"

#include "tinyfsm.hpp"
#include "state_with_context.h"
#include "flash_storage.h"
#include "module_messages.h"

namespace fk {

class ModuleState;

using ModuleFsm = tinyfsm::Fsm<ModuleState>;

using Deferred = ModuleFsm::Deferred;

class Pool;
class ModuleInfo;
class Leds;
class Watchdog;
class TwoWireBus;
class ModuleCallbacks;
class TwoWireMessageBuffer;
class TwoWireChild;
class SerialFlashFileSystem;
class PendingReadings;
class ModuleHardware;

struct DataCopyStatus {
    uint32_t checksum;
    phylum::BlockAddress pending;
};

struct ModuleServices {
    Pool *pool;
    ModuleInfo *info;
    Leds *leds;
    Watchdog *watchdog;
    TwoWireBus *bus;
    ModuleCallbacks *callbacks;
    TwoWireChild *child;
    SerialFlashFileSystem *flashFs;
    FlashStateService *flashState;
    PendingReadings *readings;
    ModuleHardware *hardware;
    ModuleQueryMessage *query;
    DataCopyStatus dataCopyStatus;

    void alive();
    void clear();
};

class ModuleState : public ModuleFsm {
public:
    virtual void react(tinyfsm::Event const &e);

public:
    virtual void entry() {
        log("Entered");
    }

    virtual void task() {
    }

    virtual void exit() {
    }

    virtual bool busy() {
        return true;
    }

public:
    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void info(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void trace(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void warn(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void error(const char *f, ...) const __attribute__((format(printf, 2, 3)));
};

using ModuleServicesState = StateWithContext<ModuleServices, ModuleState>;

using fsm_list = tinyfsm::FsmList<ModuleState>;

template<typename E>
void send_event(E const & event) {
    fsm_list::template dispatch<E>(event);
}

}

#endif
