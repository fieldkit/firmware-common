#ifndef FK_CORE_FSM_H_INCLUDED
#define FK_CORE_FSM_H_INCLUDED

#include <cinttypes>

#include "tinyfsm.hpp"

namespace fk {

class CoreDevice;

using CoreFsm = tinyfsm::Fsm<CoreDevice>;

class CoreDevice : public CoreFsm {
public:
    virtual void react(tinyfsm::Event const &ignored) {
    }

    virtual void entry() {
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

};

template<typename T>
class StateWithContext : public CoreDevice {
private:
    static T *services_;

public:
    static T &services() {
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
