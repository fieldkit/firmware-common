#include "file_system.h"

#include "debug.h"
#include "hardware.h"

namespace fk {

extern "C" {

static size_t debug_write_log(const fk_log_message_t *m, const char *formatted, void *arg) {
    EmptyPool empty;
    DataLogMessage dlm{ m, empty };
    uint8_t buffer[dlm.calculateSize()];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, dlm.forEncode())) {
        debug_uart_get()->println("Unable to encode log message");
        return 0;
    }

    if (!fkfs_log_append_binary((fkfs_log_t *)arg, buffer, stream.bytes_written)) {
        debug_uart_get()->println("Unable to append log");
    }

    return 0;
}

static size_t fkfs_log_message(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debug_configure_hook(false);
    vdebugfpln(LogLevels::TRACE, "fkfs", f, args);
    debug_configure_hook(true);
    va_end(args);
    return 0;
}

}

FileSystem::FileSystem(TwoWireBus &bus, Pool &pool) : data{ fs, bus, FKFS_FILE_DATA, pool }, replies{ fs, FKFS_FILE_DATA } {
}

bool FileSystem::setup() {
    fkfs_configure_logging(fkfs_log_message);

    if (!fkfs_create(&fs)) {
        return false;
    }

    if (!sd_raw_initialize(&fs.sd, Hardware::SD_PIN_CS)) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_LOG, FKFS_FILE_PRIORITY_LOWEST, true, "FK.LOG")) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_DATA, FKFS_FILE_PRIORITY_HIGHEST, true, "DATA.BIN")) {
        return false;
    }

    auto wipe = false;
    if (!fkfs_initialize(&fs, wipe)) {
        return false;
    }

    fkfs_log_statistics(&fs);

    if (!fkfs_log_initialize(&fkfs_log, &fs, FKFS_FILE_LOG)) {
        return false;
    }

    // This ensures our first line is on a newline by itself.
    fkfs_log_flush(&fkfs_log);

    debug_add_hook(debug_write_log, &fkfs_log);
    debug_configure_hook(true);

    return true;
}

}
