#include "take_gps_reading.h"

#include "gps.h"
#include "take_readings.h"

namespace fk {

void TakeGpsReading::task() {
    services().gps->save();

    transit(services().states->readings);
}

}
