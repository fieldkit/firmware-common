#include "wifi_client.h"

namespace fk {

WifiWriter::WifiWriter(WiFiClient &wcl) : wcl_(&wcl) {
}

int32_t WifiWriter::write(uint8_t *ptr, size_t size) {
    if (!wcl_->connected()) {
        return Stream::EOS;
    }
    return wcl_->write(ptr, size);
}

int32_t WifiWriter::write(uint8_t byte) {
    if (!wcl_->connected()) {
        return Stream::EOS;
    }
    return wcl_->write(byte);
}

void WifiWriter::close() {
}

}

