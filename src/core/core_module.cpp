#include "core_module.h"
#include "startup.h"

namespace fk {

CoreModule::CoreModule(ConfigurableStates configurableStates) : configurableStates_(configurableStates) {
}

void CoreModule::run() {
    MainServicesState::services(mainServices);
    WifiServicesState::services(wifiServices);

    fsm_list::start();

    while (true) {
        CoreDevice::current().task();
    }
}

}

