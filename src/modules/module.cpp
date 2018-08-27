#include "module.h"

namespace fk {

constexpr const char Log[] = "Module";

using Logger = SimpleLog<Log>;

Module::Module(TwoWireBus &bus, ModuleInfo &info) :
    bus_(&bus), info_(&info), twoWireChild_(bus, info.address) {
}

void Module::begin() {
    ModuleServicesState::services(moduleServices_);

    fsm_list::start();
}

void Module::tick() {
    ModuleState::current().task();
}

}
