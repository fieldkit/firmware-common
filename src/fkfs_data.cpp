#include "fkfs_data.h"

#include "debug.h"

namespace fk {

FkfsData::FkfsData(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

bool FkfsData::appendReading(SensorReading &reading) {
    if (!fkfs_file_append(fs, file, sizeof(SensorReading), (uint8_t *)&reading)) {
        debugfpln("Data", "Error appending reading.");
        return false;
    }

    debugfpln("Data", "Appended reading file %d 0x%x (%d)", file, fs, sizeof(SensorReading));
    return true;
}

}
