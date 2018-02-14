#include "file_system.h"

#include "debug.h"
#include "hardware.h"

namespace fk {

extern "C" {

static size_t debug_write_log(const char *str, void *arg) {
    if (!fkfs_log_append((fkfs_log_t *)arg, str)) {
        debug_uart_get()->println("Unable to append log");
    }
    return 0;
}

static size_t fkfs_log_message(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debug_configure_hook(false);
    vdebugfpln("fkfs", f, args);
    debug_configure_hook(true);
    va_end(args);
    return 0;
}

}

FileSystem::FileSystem(TwoWireBus &bus, Pool &pool) : data{ fs, bus, FKFS_FILE_DATA, pool } {
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
    fkfs_log_append(&fkfs_log, "\n\nStartup\n\n");
    fkfs_log_flush(&fkfs_log);

    debug_add_hook(debug_write_log, &fkfs_log);
    debug_configure_hook(true);

    return true;
}

}
