#ifndef FK_SLEEP_H_INCLUDED
#define FK_SLEEP_H_INCLUDED

#include "state_services.h"
#include "file_system.h"

namespace fk {

class Sleep : public MainServicesState {
private:
    uint32_t maximum_{ 0 };
    uint32_t activity_{ 0 };

public:
    Sleep() {
    }

    Sleep(uint32_t maximum) : maximum_(maximum) {
    }

public:
    const char *name() const override {
        return "Sleep";
    }

public:
    void entry() override;
    void react(UserButtonEvent const &ignored) override;
    void task() override;

protected:
    void sleep(uint32_t maximum);

};

}

#endif
