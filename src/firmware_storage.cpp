#include "firmware_storage.h"

namespace fk {

FirmwareStorage::FirmwareStorage() {
}

lws::Writer *FirmwareStorage::write() {
    return nullptr;
}

lws::Reader *FirmwareStorage::read() {
    return nullptr;
}

}
