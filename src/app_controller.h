#ifndef FK_APP_CONTROLLER_H_INCLUDED
#define FK_APP_CONTROLLER_H_INCLUDED

#include "active_object.h"
#include "module_controller.h"
#include "app_messages.h"

namespace fk {

class AppController : public ActiveObject {
private:
    ModuleController *modules;

public:
    AppController(ModuleController *modules);

public:
    void handle(AppQueryMessage &query);

};

}

#endif
