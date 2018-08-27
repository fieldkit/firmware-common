#include "tuning.h"
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

void Module::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    loginfof("Module", f, args);
    va_end(args);
}

ModuleReadingStatus Module::beginReading(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

Deferred Module::beginReadingState() {
    return Deferred{ };
}

ModuleReadingStatus Module::readingStatus(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

}
