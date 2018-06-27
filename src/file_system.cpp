#include "file_system.h"

#include "debug.h"
#include "hardware.h"
#include "rtc.h"

using namespace phylum;

namespace fk {

static phylum::SimpleFile logFile;
static phylum::SimpleFile dataFile;

extern "C" {

static uint32_t log_uptime() {
    return clock.getTime();
}

static size_t debug_write_log(const LogMessage *m, const char *formatted, void *arg) {
    if (m->level == (uint8_t)LogLevels::TRACE) {
        return 0;
    }

    EmptyPool empty;
    DataLogMessage dlm{ m, empty };
    uint8_t buffer[dlm.calculateSize()];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, dlm.forEncode())) {
        log_uart_get()->println("Unable to encode log message");
        return 0;
    }

    int32_t bytes = stream.bytes_written;
    if (logFile.write(buffer, bytes) != bytes) {
        log_uart_get()->println("Unable to append log");
        return 0;
    }

    return 0;
}

}

FileSystem::FileSystem(TwoWireBus &bus, Pool &pool) : data_{ bus, files_, pool }, replies_{ } {
}

bool FileSystem::setup() {
    if (!storage.initialize(g, Hardware::SD_PIN_CS)) {
        return false;
    }

    if (!storage.open()) {
        return false;
    }

    if (!fs.mount(files_.descriptors)) {
        if (!fs.format(files_.descriptors)) {
            return false;
        }
        if (!fs.mount(files_.descriptors)) {
            return false;
        }
    }

    auto startup = fs.open(files_.file_log_startup_fd, OpenMode::Write);
    if (!startup) {
        return false;
    }

    logFile = startup;

    dataFile = fs.open(files_.file_data_fk, OpenMode::Write);
    if (!dataFile) {
        return false;
    }

    log_configure_time(millis, log_uptime);
    log_add_hook(debug_write_log, nullptr);
    log_configure_hook(true);

    return true;
}

bool FileSystem::openData() {
    files_.opened = fs.open(files_.file_data_fk, OpenMode::Read);
    if (!files_.opened) {
        return false;
    }

    if (!files_.opened.seek(UINT64_MAX)) {
        return false;
    }

    files_.reader_ = FileReader{ &files_.opened };

    return true;
}

Files::Files(phylum::FileOpener &files) : files(&files) {
}

phylum::SimpleFile &Files::log() {
    return logFile;
}

phylum::SimpleFile &Files::data() {
    return dataFile;
}

FileReader &Files::reader() {
    return reader_;
}

}
