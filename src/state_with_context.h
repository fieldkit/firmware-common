#ifndef FK_STATE_WITH_CONTEXT_H_INCLUDED
#define FK_STATE_WITH_CONTEXT_H_INCLUDED

#include "platform.h"

namespace fk {

template<typename T, typename B>
class StateWithContext : public B {
private:
    static T *services_;
    uint32_t entered_{ 0 };

public:
    uint32_t elapsed() {
        return fk_uptime() - entered_;
    }

public:
    void entry() override {
        B::entry();
        services_->clear();
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

}

#endif
