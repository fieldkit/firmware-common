#include "restart_wizard.h"
#include "asf.h"

namespace fk {

RestartWizard restartWizard __attribute__ ((section (".noinit")));

static bool cygEnabled = false;

void RestartWizard::startup() {
    if (system_get_reset_cause() != SYSTEM_RESET_CAUSE_WDT) {
        debugfpln("RW", "RestartWizard: Reset.");
        history.startup();
        calls.startup();
    }
    else {
        dump();
    }
    cygEnabled = true;
}

void RestartWizard::checkin(Call where) {
    calls.checkin(where);
}

void RestartWizard::checkin(const char *where) {
    history.checkin(where);
}

void RestartWizard::dump() {
    debugfpln("RW", "History:");
    history.dump([] (size_t i, const char *entry) {
        if (entry != nullptr) {
            debugfpln("RW", "[%02d]: %s", i, entry);
        }
    });

    debugfpln("RW", "Calls:");
    calls.dump([] (size_t i, Call &entry) {
        debugfpln("RW", "[%02d]: %p %p", i, entry.function, entry.site);
    });
}

}

extern "C" {
    void __cyg_profile_func_enter(void *thisFn, void *callSite) __attribute__((no_instrument_function));

    void __cyg_profile_func_exit(void *thisFn, void *callSite) __attribute__((no_instrument_function));

    void __cyg_profile_func_enter(void *thisFn, void *callSite) {
        if (fk::cygEnabled) {
            fk::cygEnabled = false;
            fk::restartWizard.checkin(fk::Call{ thisFn, callSite });
            fk::cygEnabled = true;
        }
    }

    void __cyg_profile_func_exit(void *thisFn, void *callSite) {
    }
}
