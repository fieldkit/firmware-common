#include "check_firmware.h"
#include "transmit_files.h"

namespace fk {

void CheckFirmware::task() {
    transit<WifiTransmitFiles>();
}

}
