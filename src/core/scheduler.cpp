#include <alogging/sprintf.h>

#include "scheduler.h"
#include "core_fsm.h"

namespace fk {

void TaskLogger::visit(lwcron::PeriodicTask &task) {
    sdebug() << "PeriodicTask<'" << task.toString() << "' every " << task.interval() << "s" << (task.valid() ? "" : " INVALID") << ">" << endl;
}

void TaskLogger::visit(lwcron::CronTask &task) {
    char buffer[3 * 2 + 8 * 2 + 8 * 2 + 3 + 3 + 3 + 2 + 1 + 8];

    auto s = task.spec();
    alogging_snprintf(buffer, sizeof(buffer), "S(%02x%02x%02x%02x%02x%02x%02x%02x) M(%02x%02x%02x%02x%02x%02x%02x%02x) H(%02x%02x%02x)",
                      s.seconds[0], s.seconds[1], s.seconds[2], s.seconds[3],
                      s.seconds[4], s.seconds[5], s.seconds[6], s.seconds[7],
                      s.minutes[0], s.minutes[1], s.minutes[2], s.minutes[3],
                      s.minutes[4], s.minutes[5], s.minutes[6], s.minutes[7],
                      s.hours[0], s.hours[1], s.hours[2]
        );

    sdebug() << "CronTask<'" << task.toString() << "' " << buffer << (task.valid() ? "" : " INVALID") << ">" << endl;
}

}
