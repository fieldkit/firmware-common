#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "module_controller.h"
#include "app_messages.h"

namespace fk {

class AppServicer : public Task {
private:
    ModuleController *modules;

public:
    AppServicer(ModuleController *modules);

public:
    TaskEval &task() override;

    void handle(AppQueryMessage &query);

};

}

#endif
