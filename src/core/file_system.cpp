#include "debug.h"
#include "hardware.h"
#include "file_system.h"
#include "file_cursors.h"
#include "rtc.h"

using namespace phylum;

namespace fk {

static Files *global_files{ nullptr };

constexpr const char Log[] = "FileSystem";

using Logger = SimpleLog<Log>;

extern "C" {

static uint32_t log_uptime() {
    return clock.getTime();
}

static size_t debug_write_log(const LogMessage *m, const char *formatted, void *arg) {
    if (m->level == (uint8_t)LogLevels::TRACE) {
        return 0;
    }

    if (!Hardware::peripheralsEnabled()) {
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

    auto &log = global_files->log();
    if (log) {
        int32_t bytes = stream.bytes_written;
        if (log.write(buffer, bytes, true) != bytes) {
            log_uart_get()->println("Unable to append log");
            return 0;
        }

        global_files->swapLogsIfNecessary();
    }

    return 0;
}

}

FileSystem::FileSystem() : data_{ files_ }, replies_{ *this } {
}

bool FileSystem::formatAll() {
    Logger::info("Formatting SD FileSystem...");

    if (formatted_) {
        Logger::error("Already formatted, failing.");
        return false;
    }

    formatted_ = true;

    if (!storage_.initialize(g_, Hardware::SD_PIN_CS)) {
        Logger::error("Unable to initialize SD.");
        return false;
    }

    if (!fs_.format(files_.descriptors_)) {
        Logger::error("Format failed!");
        return false;
    }

    if (!fs_.mount(files_.descriptors_)) {
        Logger::error("Mount failed!");
        return false;
    }

    return true;
}

bool FileSystem::eraseAll() {
    auto success = true;

    closeSystemFiles();

    for (auto &fd : files_.descriptors_) {
        if (!fs_.erase(*fd)) {
            success = false;
            Logger::error("Erase failed: %s", fd->name);
        }
    }

    if (!openSystemFiles()) {
        return false;
    }

    return success;
}

bool FileSystem::setup() {
    if (!storage_.initialize(g_, Hardware::SD_PIN_CS)) {
        Logger::error("Unable to initialize SD.");
        return false;
    }

    if (!storage_.open()) {
        Logger::error("Unable to open SD.");
        return false;
    }

    for (auto i = 0; i < 2; ++i) {
        if (!fs_.mount(files_.descriptors_)) {
            Logger::error("Mount failed!");

            if (!formatAll()) {
                return false;
            }
        }

        Logger::info("Mounted (%lu blocks)", storage_.geometry().number_of_blocks);

        if (!openSystemFiles()) {
            if (!formatAll()) {
                return false;
            }
        }
        else {
            break;
        }
    }

    log_configure_time(fk_uptime, log_uptime);
    log_configure_hook_register(debug_write_log, nullptr);
    log_configure_hook(true);

    return true;
}

bool FileSystem::closeSystemFiles() {
    if (files_.log_) {
        files_.log_.close();
    }
    if (files_.data_) {
        files_.data_.close();
    }
    return true;
}

static SimpleFile open_file_or_truncate(FileOpener &opener, phylum::FileDescriptor &fd) {
    auto file = opener.open(fd, OpenMode::MultipleWrites);
    if (!file) {
        Logger::error("Opening %s failed, erasing...", fd.name);

        if (!opener.erase(fd)) {
            Logger::error("Erase %s failed!", fd.name);
            return { };
        }

        return opener.open(fd, OpenMode::MultipleWrites);
    }

    return file;
}

bool FileSystem::openSystemFiles() {
    auto logs_a = open_file_or_truncate(fs_, files_.file_logs_a_fd);
    if (!logs_a) {
        return false;
    }

    auto logs_b = open_file_or_truncate(fs_, files_.file_logs_b_fd);
    if (!logs_b) {
        return false;
    }

    auto data = open_file_or_truncate(fs_, files_.file_data_fk);
    if (!data) {
        return false;
    }

    files_.data_ = data;


    if (logs_a.in_final_block()) {
        files_.log_ = logs_b;
    }
    else {
        files_.log_ = logs_a;
    }

    // Report file size information.
    for (auto fd : files_.descriptors_) {
        auto opened = fs_.open(*fd);
        if (opened) {
            Logger::info("File: %s size = %lu maximum = %lu", fd->name, (uint32_t)opened.size(), (uint32_t)fd->maximum_size);
        }
    }

    return true;
}

bool FileSystem::erase(FileNumber number) {
    auto fd = files_.descriptors_[(size_t)number];
    auto success = true;

    closeSystemFiles();

    if (!fs_.erase(*fd)) {
        success = false;
        Logger::error("Erase failed: %s", fd->name);
    }

    if (!openSystemFiles()) {
        return false;
    }

    return success;
}

bool FileSystem::beginFileCopy(FileCopySettings settings) {
    auto fd = files_.descriptors_[(size_t)settings.file];

    files_.opened_ = fs_.open(*fd, OpenMode::Read);
    if (!files_.opened_) {
        return false;
    }

    auto newReader = FileReader{ files_.opened_ };
    if (!files_.fileCopy_.prepare(newReader, settings)) {
        return false;
    }

    Logger::info("Prepare: id=%d name=%s offset=%lu length=%lu fileSize=%lu readerSize=%d",
                 (size_t)settings.file, fd->name,
                 settings.offset, settings.length,
                 (uint32_t)files_.opened_.size(), files_.fileCopy_.size());

    return true;
}

bool FileSystem::flush() {
    if (!files_.log_.flush()) {
        return false;
    }

    if (!files_.data_.flush()) {
        return false;
    }

    return true;
}

phylum::SimpleFile FileSystem::openSystem(phylum::OpenMode mode) {
    return fs_.open(files_.file_system_area_fd, mode);
}

Files::Files(phylum::FileOpener &files) : files_(&files) {
    global_files = this;
}

bool Files::swapLogsIfNecessary() {
    if (!log_.in_final_block()) {
        return true;
    }

    auto new_fd = &file_logs_a_fd;
    if (&log_.fd() == &file_logs_a_fd) {
        new_fd = &file_logs_b_fd;
    }

    if (!files_->erase(*new_fd)) {
        return false;
    }

    auto other = files_->open(*new_fd, phylum::OpenMode::MultipleWrites);
    if (!other) {
        return false;
    }

    log_.close();

    log_ = other;

    return true;
}

phylum::SimpleFile &Files::log() {
    return log_;
}

phylum::SimpleFile &Files::data() {
    return data_;
}

FileNumber Files::logFileNumber() {
    if (&log_.fd() == &file_logs_a_fd) {
        return FileNumber::LogsA;
    }
    return FileNumber::LogsB;
}

FileCopyOperation &Files::fileCopy() {
    return fileCopy_;
}

}
