#ifndef FK_CORE_FSM_H_INCLUDED
#define FK_CORE_FSM_H_INCLUDED

#include "tinyfsm.hpp"

namespace fk {

class CoreDevice;

using CoreFsm = tinyfsm::Fsm<CoreDevice>;

class CoreDevice : public CoreFsm {
public:
    void react(tinyfsm::Event const &) {
    };

    virtual void entry(void) {
    };

    void exit(void) {
    };

};

using fsm_list = tinyfsm::FsmList<CoreDevice>;

template<typename E>
void send_event(E const & event) {
    fsm_list::template dispatch<E>(event);
}

}

#endif
