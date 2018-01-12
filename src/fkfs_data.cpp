#include "fkfs_data.h"

#include "debug.h"

namespace fk {

constexpr const char Log[] = "Data";

FkfsData::FkfsData(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

bool FkfsData::appendLocation(DeviceLocation &location) {
    auto entry = DataEntry{location};

    if (!fkfs_file_append(fs, file, sizeof(DataEntry), (uint8_t *)&entry)) {
        debugfpln("Data", "Error appending data file.");
        return false;
    }

    debugfpln(Log, "Appended location.");
    return true;
}

bool FkfsData::appendReading(DeviceLocation &location, SensorReading &reading) {
    auto entry = DataEntry{location, reading};

    if (!fkfs_file_append(fs, file, sizeof(DataEntry), (uint8_t *)&entry)) {
        debugfpln("Data", "Error appending data file.");
        return false;
    }

    debugfpln(Log, "Appended reading (%lu, %f)", reading.time, reading.value);
    return true;
}

}
