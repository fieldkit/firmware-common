#ifndef FK_CORE_FSM_H_INCLUDED
#define FK_CORE_FSM_H_INCLUDED

#include <cinttypes>

#include "tinyfsm.hpp"

#include "platform.h"
#include "debug.h"

namespace fk {

struct LiveDataEvent : public tinyfsm::Event {
    uint32_t interval;

    LiveDataEvent(uint32_t interval) : interval(interval) {
    }
};

struct AppQueryEvent : public tinyfsm::Event {
    uint8_t type;

    AppQueryEvent(uint8_t type) : type(type) {
    }
};

struct WifiEvent : public tinyfsm::Event {
    bool enabled;

    WifiEvent(bool enabled) : enabled(enabled) {
    }
};

struct LowPowerEvent : public tinyfsm::Event {
};

struct UserWakeupEvent : public tinyfsm::Event {
};

struct UserRebootEvent : public tinyfsm::Event {
};

class CoreDevice;

using CoreFsm = tinyfsm::Fsm<CoreDevice>;

struct SchedulerEvent : public tinyfsm::Event {
    CoreFsm::Deferred deferred;

    SchedulerEvent(CoreFsm::Deferred deferred) : deferred(deferred) {
    }

    const char *toString() const {
        if (deferred) {
            return deferred.name();
        }
        return "<none>";
    }
};

class CoreDevice : public CoreFsm {
public:
    virtual void react(tinyfsm::Event const &ignored);
    virtual void react(LowPowerEvent const &ignored);
    virtual void react(SchedulerEvent const &ignored);
    virtual void react(LiveDataEvent const &ignored);
    virtual void react(AppQueryEvent const &ignored);
    virtual void react(UserWakeupEvent const &ignored);
    virtual void react(UserRebootEvent const &ignored);

public:
    virtual void entry() {
        log("Entered");
    }

    virtual void task() {
    }

    virtual void exit() {
    }

public:
    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void info(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void trace(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void warn(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void error(const char *f, ...) const __attribute__((format(printf, 2, 3)));

};

template<typename T>
class StateWithContext : public CoreDevice {
private:
    static T *services_;
    uint32_t entered_{ 0 };

public:
    uint32_t elapsed() {
        return fk_uptime() - entered_;
    }

public:
    void entry() override {
        CoreDevice::entry();
        entered_ = fk_uptime();
    }

public:
    static T &services() {
        fk_assert(services_ != nullptr);
        return *services_;
    }

    static void services(T &newServices) {
        services_ = &newServices;
    }

};

using fsm_list = tinyfsm::FsmList<CoreDevice>;

template<typename E>
void send_event(E const & event) {
    fsm_list::template dispatch<E>(event);
}

}

#endif
