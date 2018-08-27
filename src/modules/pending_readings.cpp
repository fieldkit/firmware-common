#include "pending_readings.h"
#include "rtc.h"

namespace fk {

PendingReadings::PendingReadings(ModuleInfo &info) : info_(&info) {
}

void PendingReadings::done(size_t i, float value) {
    assert(i < info_->numberOfSensors);

    info_->readings[i].time = clock.getTime();
    info_->readings[i].value = value;
    info_->readings[i].status = SensorReadingStatus::Done;
}

}
