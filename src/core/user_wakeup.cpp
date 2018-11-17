#include "user_wakeup.h"
#include "file_system.h"
#include "wifi_startup.h"

namespace fk {

void UserWakeup::task() {
    services().fileSystem->flush();

    transit_into<WifiStartup>(WifiCheckConfig{ true });
}

}
