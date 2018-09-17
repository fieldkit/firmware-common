#include "begin_gather_readings.h"

#include "take_gps_reading.h"

namespace fk {

void BeginGatherReadings::task() {
    resume_at_back();

    transit<TakeGpsReading>();
}

}
