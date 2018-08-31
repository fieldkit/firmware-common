#include "core_module.h"
#include "startup.h"

namespace fk {

void CoreModule::run(tinyfsm::Fsm<fk::CoreDevice>::Deferred configuration) {
    MainServicesState::services(mainServices);
    WifiServicesState::services(wifiServices);

    StartSystem::configureState(configuration);

    fsm_list::start();

    while (true) {
        CoreDevice::current().task();
    }
}

}

