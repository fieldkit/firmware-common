#include <WiFi101.h>

#include "check_firmware.h"
#include "transmit_files.h"

#include "http_response_parser.h"
#include "http_response_writer.h"

#include "firmware_header.h"
#include "firmware_storage.h"

namespace fk {

void erase(SerialFlashChip &serialFlash) {
    auto starting = FLASH_FIRMWARE_BANK_1_ADDRESS;
    auto block_size = serialFlash.blockSize();

    for (auto i = 0; i < 8; ++i) {
        auto address = starting + i * block_size;
        serialFlash.eraseBlock(address);
    }
}

void CheckAllAttachedFirmware::task() {
    auto state = services().state;

    if (index_ < state->numberOfModules()) {
        auto module = state->getModuleByIndex(index_);
        index_++;
        transit_into<CheckFirmware>(FirmwareBank::ModuleA, module->module);
    }
    else if (index_ == state->numberOfModules()) {
        index_++;
        transit_into<CheckFirmware>(FirmwareBank::CoreA, "fk-core");
    }
    else {
        transit<WifiTransmitFiles>();
    }
}

void CheckFirmware::task() {
    if (module_ == nullptr) {
        log("No module given");
        back();
        return;
    }

    check();
}

void CheckFirmware::check() {
    fk::Url parsed(WifiApiUrlFirmware, deviceId.toString(), module_);
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };
    HttpResponseParser parser;
    WiFiClient wcl;

    log("GET http://%s:%d/%s", parsed.server, parsed.port, parsed.path);

    firmware_header_t header;

    if (firmwareStorage.header(bank_, header)) {
        if (header.version != FIRMWARE_VERSION_INVALID) {
            log("Have: %s", header.etag);
        }
    }

    if (wcl.connect(parsed.server, parsed.port)) {
        OutgoingHttpHeaders headers{
            nullptr,
            firmware_version_get(),
            firmware_build_get(),
            deviceId.toString(),
            header.version != FIRMWARE_VERSION_INVALID ? header.etag : nullptr,
        };
        HttpHeadersWriter httpWriter(wcl);
        HttpResponseParser httpParser;

        log("Connected!");

        httpWriter.writeHeaders(parsed, "GET", headers);

        auto writer = (lws::Writer *)nullptr;
        auto started = millis();
        auto total = (uint32_t)0;

        while (wcl.connected() || wcl.available()) {
            delay(10);

            if (millis() - started > WifiConnectionTimeout) {
                error("Failed!");
                break;
            }

            while (wcl.available()) {
                if (httpParser.reading_header()) {
                    httpParser.write(wcl.read());
                }
                else {
                    uint8_t buffer[WifiSocketBufferSize];

                    auto bytes = wcl.read(buffer, sizeof(buffer));
                    if (bytes > 0) {
                        if (httpParser.status_code() == 200) {
                            if (writer == nullptr) {
                                writer = firmwareStorage.write();

                                firmware_header_t header;
                                header.version = 1;
                                header.position = 0;
                                header.size = httpParser.content_length();
                                strncpy(header.etag, httpParser.etag(), sizeof(header.etag));

                                if (writer->write((uint8_t *)&header, sizeof(firmware_header_t)) != sizeof(firmware_header_t)) {
                                    error("Writing header failed.");
                                }
                            }
                            writer->write(buffer, bytes);
                            total += bytes;
                        }
                    }
                }
            }
        }

        if (total > 0 && writer != nullptr) {
            if (total != httpParser.content_length()) {
                error("Status: %d (Size mismatch!) total=%lu expected=%lu etag='%s'", httpParser.status_code(),
                      total, httpParser.content_length(), httpParser.etag());
            }
            else {
                log("Status: %d total=%lu etag='%s'", httpParser.status_code(), total, httpParser.etag());

                firmwareStorage.update(bank_, writer, httpParser.etag());
            }
        }
        else {
            log("Status: %d", httpParser.status_code());
        }

        wcl.stop();

        log("Done!");
    }
    else {
        error("Connection failed!");
    }

    back();
}

}
