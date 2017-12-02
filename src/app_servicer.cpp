#include "app_servicer.h"

namespace fk {

AppServicer::AppServicer(ModuleController *modules) :
    Task("AppServicer"), modules(modules) {
}

TaskEval &AppServicer::task() {
    return TaskEval::Idle;
}

void AppServicer::handle(AppQueryMessage &query) {
}

}
