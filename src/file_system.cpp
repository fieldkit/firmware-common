#include "debug.h"
#include "hardware.h"
#include "file_system.h"
#include "file_cursors.h"
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

FileSystem::FileSystem(TwoWireBus &bus) : data_{ bus, files_ }, replies_{ *this } {
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

bool FileSystem::eraseAll() {
    auto success = true;

    closeSystemFiles();

    for (auto &fd : files_.descriptors_) {
        if (!fs_.erase(*fd)) {
            success = false;
            log("Erase failed");
        }
    }

    if (!openSystemFiles()) {
        return false;
    }

    return success;
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

    log("Mounted");

    if (!openSystemFiles()) {
        return false;
    }

    log_configure_time(fk_uptime, log_uptime);
    log_add_hook(debug_write_log, nullptr);
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

bool FileSystem::openSystemFiles() {
    auto logs_a = fs_.open(files_.file_logs_a_fd, OpenMode::MultipleWrites);
    if (!logs_a) {
        return false;
    }

    auto logs_b = fs_.open(files_.file_logs_b_fd, OpenMode::MultipleWrites);
    if (!logs_b) {
        return false;
    }

    auto data = fs_.open(files_.file_data_fk, OpenMode::MultipleWrites);
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

    for (auto fd : files_.descriptors_) {
        auto opened = fs_.open(*fd);
        if (opened) {
            log("File: %s size = %lu maximum = %lu", fd->name, (uint32_t)opened.size(), (uint32_t)fd->maximum_size);
            opened.close();
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
        log("Erase failed: %d", number);
    }

    if (!openSystemFiles()) {
        return false;
    }

    return success;
}

bool FileSystem::beginFileCopy(FileCopySettings settings) {
    auto fd = files_.descriptors_[(size_t)settings.file];

    log("Prepare: id=%d name=%s offset=%lu length=%lu",
         (size_t)settings.file, fd->name, settings.offset, settings.length);

    files_.opened_ = fs_.open(*fd, OpenMode::Read);
    if (!files_.opened_) {
        return false;
    }

    auto newReader = FileReader{ &files_.opened_ };
    if (!files_.fileCopy_.prepare(newReader, settings)) {
        return false;
    }

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

void FileSystem::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, Log, f, args);
    va_end(args);
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

FileCopyOperation::FileCopyOperation() {
}

bool FileCopyOperation::prepare(const FileReader &reader, const FileCopySettings &settings) {
    reader_ = reader;

    streamCopier_.restart();

    if (!reader_.open(settings.offset, settings.length)) {
        return false;
    }

    started_ = 0;
    busy_ = true;
    copied_ = 0;
    lastStatus_ = fk_uptime();
    total_ = reader_.size() - reader_.tell();

    return true;
}

bool FileCopyOperation::copy(lws::Writer &writer, FileCopyCallbacks *callbacks) {
    if (started_ == 0) {
        started_ = fk_uptime();
    }

    auto started = fk_uptime();
    while (fk_uptime() - started < FileCopyMaximumElapsed) {
        if (reader_.isFinished()) {
            status();
            busy_ = false;
            return true;
        }

        auto bytes = streamCopier_.copy(reader_, writer);
        if (bytes == 0) {
            break;
        }

        if (bytes > 0) {
            copied_ += bytes;
        }

        if (bytes == lws::Stream::EOS) {
            status();
            busy_ = false;
            return false;
        }

        if (fk_uptime() - lastStatus_ > FileCopyStatusInterval) {
            status();
            lastStatus_ = fk_uptime();
        }

        if (callbacks != nullptr) {
            callbacks->fileCopyTick();
        }
    }

    return true;
}

void FileCopyOperation::status() {
    auto elapsed = fk_uptime() - started_;
    auto complete = copied_ > 0 ? ((float)copied_ / total_) * 100.0f : 0.0f;
    auto speed = copied_ > 0 ? copied_ / ((float)elapsed / 1000.0f) : 0.0f;
    logf(LogLevels::TRACE, "Copy", "%lu/%lu %lums %.2f %.2fbps (%lu)",
         copied_, total_, elapsed, complete, speed, fk_uptime() - lastStatus_);
}

}
