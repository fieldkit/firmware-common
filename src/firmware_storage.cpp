#include "firmware_storage.h"

namespace fk {

FirmwareStorage::FirmwareStorage() {
}

lws::Writer *FirmwareStorage::write(FirmwareBank bank) {
    return nullptr;
}

lws::Reader *FirmwareStorage::read(FirmwareBank bank) {
    return nullptr;
}

}
