#include "file_system.h"
#include "debug.h"
#include "hardware.h"
#include "rtc.h"

using namespace phylum;

namespace fk {

static Files *global_files{ nullptr };

constexpr const char Log[] = "FileSystem";

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
    if (global_files->log().write(buffer, bytes, true) != bytes) {
        log_uart_get()->println("Unable to append log");
        return 0;
    }

    return 0;
}

}

FileSystem::FileSystem(TwoWireBus &bus, Pool &pool) : data_{ bus, files_, pool }, replies_{ *this } {
}

bool FileSystem::format() {
    if (!fs_.format(files_.descriptors_)) {
        logf(LogLevels::ERROR, Log, "Format failed!");
        return false;
    }

    if (!fs_.mount(files_.descriptors_)) {
        logf(LogLevels::ERROR, Log, "Mount failed!");
        return false;
    }

    logf(LogLevels::INFO, Log, "Formatted!");

    return true;
}

bool FileSystem::setup() {
    if (!storage_.initialize(g_, Hardware::SD_PIN_CS)) {
        logf(LogLevels::ERROR, Log, "Unable to initialize SD.");
        return false;
    }

    if (!storage_.open()) {
        logf(LogLevels::ERROR, Log, "Unable to open SD.");
        return false;
    }

    if (!fs_.mount(files_.descriptors_)) {
        logf(LogLevels::ERROR, Log, "Mount failed!");

        if (!format()) {
            return false;
        }
    }

    logf(LogLevels::INFO, Log, "Mounted");

    auto startup = fs_.open(files_.file_log_startup_fd, OpenMode::Write);
    if (!startup) {
        return false;
    }

    files_.log_ = startup;

    files_.data_ = fs_.open(files_.file_data_fk, OpenMode::Write);
    if (!files_.data_) {
        return false;
    }

    log_configure_time(millis, log_uptime);
    log_add_hook(debug_write_log, nullptr);
    log_configure_hook(true);

    return true;
}

bool FileSystem::openForReading(uint8_t file) {
    files_.opened_ = fs_.open(files_.file_data_fk, OpenMode::Read);
    if (!files_.opened_) {
        return false;
    }

    files_.reader_ = FileReader{ &files_.opened_ };

    return true;
}

Files::Files(phylum::FileOpener &files) : files_(&files) {
    global_files = this;
}

phylum::SimpleFile &Files::log() {
    return log_;
}

phylum::SimpleFile &Files::data() {
    return data_;
}

FileReader &Files::reader() {
    return reader_;
}

}
