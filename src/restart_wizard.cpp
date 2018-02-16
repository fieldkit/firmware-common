#include "restart_wizard.h"
#include "asf.h"

namespace fk {

RestartWizard restartWizard __attribute__ ((section (".noinit")));

static bool cygEnabled = false;

void RestartWizard::startup() {
    if (system_get_reset_cause() != SYSTEM_RESET_CAUSE_WDT) {
        debugfpln("RW", "RestartWizard: Reset.");
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

void RestartWizard::dump() {
    debugfpln("RW", "LastLoop: %lu", lastLoop);
    debugfpln("RW", "Calls:");
    calls.dump([] (size_t i, Call &entry) {
        debugfpln("RW", "[%02d]: %p %p %d", i, entry.function, entry.site, entry.returned);
    });
}

}

#include "driver/source/nmbus.h"
#include "driver/include/m2m_periph.h"
#include "driver/source/m2m_hif.h"

extern "C" {
    static bool masked = false;

    bool isMasked(void *ptr)  __attribute__((no_instrument_function));

    bool isMasked(void *ptr) {
        if (ptr == m2m_periph_gpio_set_val) {
            return true;
        }
        if (ptr == nm_read_block || ptr == nm_read_reg_with_ret || ptr == nm_write_reg) {
            return true;
        }
        if (ptr == hif_receive) {
            return true;
        }

        return false;
    }

    void __cyg_profile_func_enter(void *thisFn, void *callSite) __attribute__((no_instrument_function));

    void __cyg_profile_func_exit(void *thisFn, void *callSite) __attribute__((no_instrument_function));

    void __cyg_profile_func_enter(void *thisFn, void *callSite) {
        if (fk::cygEnabled) {
            if (!masked) {
                fk::cygEnabled = false;
                fk::restartWizard.checkin(fk::Call{ thisFn, callSite });
                fk::cygEnabled = true;

                if (isMasked(thisFn)) {
                    masked = true;
                }
            }
        }
    }

    void __cyg_profile_func_exit(void *thisFn, void *callSite) {
        if (fk::cygEnabled) {
            if (masked) {
                if (isMasked(thisFn)) {
                    masked = false;
                }
            }

            fk::cygEnabled = false;
            auto lastCall = fk::restartWizard.lastCall();
            if (thisFn == lastCall.function) {
                lastCall.returned = true;
            }
            fk::cygEnabled = true;
        }
    }
}
