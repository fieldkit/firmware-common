#include <WiFi101.h>

#include "check_firmware.h"
#include "transmit_files.h"

#include "firmware_header.h"
#include "http_response_parser.h"
#include "http_response_writer.h"

namespace fk {

static void erase(SerialFlashChip &serialFlash) {
    auto starting = FLASH_FIRMWARE_BANK_1_ADDRESS;
    auto block_size = serialFlash.blockSize();

    for (auto i = 0; i < 8; ++i) {
        auto address = starting + i * block_size;
        serialFlash.eraseBlock(address);
    }
}

void CheckFirmware::task() {
    fk::Url parsed(WifiApiUrlFirmware, deviceId.toString(), "fk-core");
    SerialFlashChip serialFlash;
    HttpResponseParser parser;
    WiFiClient wcl;

    if (!serialFlash.begin(Hardware::FLASH_PIN_CS)) {
        log("Error opening serial flash");
        transit<WifiTransmitFiles>();
        return;
    }

    firmware_header_t bank1;
    serialFlash.read(FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS, &bank1, sizeof(bank1));

    if (bank1.version != FIRMWARE_VERSION_INVALID) {
        log("Bank1: version=%lu size=%lu (%s)", bank1.version, bank1.size, bank1.etag);
    }
    else {
        log("Bank1: invalid");
    }
    log("GET http://%s:%d/%s", parsed.server, parsed.port, parsed.path);

    if (wcl.connect(parsed.server, parsed.port)) {
        OutgoingHttpHeaders headers{
            nullptr,
            firmware_version_get(),
            firmware_build_get(),
            deviceId.toString(),
            bank1.version != FIRMWARE_VERSION_INVALID ? bank1.etag : nullptr,
        };
        HttpHeadersWriter httpWriter(wcl);
        HttpResponseParser httpParser;

        log("Connected!");

        httpWriter.writeHeaders(parsed, "GET", headers);

        auto started = millis();
        auto total = 0;
        auto starting = FLASH_FIRMWARE_BANK_1_ADDRESS;
        auto address = starting;

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
                            if (total == 0) {
                                log("Erasing");
                                erase(serialFlash);
                            }
                            serialFlash.write(address, buffer, bytes);
                            total += bytes;
                            address += bytes;
                        }
                    }
                }
            }
        }

        if (total > 0) {
            firmware_header_t header;
            header.version = 1;
            header.position = starting;
            header.size = total;
            strncpy(header.etag, httpParser.etag(), sizeof(header.etag));
            serialFlash.write(FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS, &header, sizeof(header));
            log("Status: %d total=%d etag='%s'", httpParser.status_code(), total, header.etag);
        }
        else {
            log("Status: %d", httpParser.status_code());
        }

        wcl.stop();

        log("Done! (%d bytes)", total);
    }
    else {
        error("Connection failed!");
    }

    transit<WifiTransmitFiles>();
}

}
