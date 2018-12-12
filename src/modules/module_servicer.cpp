#include "module_servicer.h"
#include "module.h"
#include "rtc.h"
#include "module_idle.h"
#include "module_receive_data.h"
#include "firmware_storage.h"
#include "module_firmware_self_flash.h"

namespace fk {

void ModuleServicer::task() {
    auto& incoming = services().child->incoming();
    auto& outgoing = services().child->outgoing();

    if (!incoming.empty()) {
        auto pos = incoming.position();
        auto status = incoming.read(*services().query);
        incoming.clear();
        if (!status) {
            log("Malformed message (%d bytes)", pos);
            transit<ModuleIdle>();
            return;
        }
        else {
            log("Received (%d bytes)", pos);
        }

        if (!outgoing.empty()) {
            log("Orphaned reply! QueryType=%d ReplySize=%d", outgoing.position(), services().query->m().type);
            outgoing.clear();
        }

        handle(*services().query);

        if (!transitioned()) {
            transit<ModuleIdle>();
        }
    }
}

void ModuleServicer::handle(ModuleQueryMessage &query) {
    auto pool = services().pool;
    auto info = services().info;
    auto& outgoing = services().child->outgoing();

    services().child->clear();

    switch (query.m().type) {
    case fk_module_QueryType_QUERY_CAPABILITIES: {
        log("Module info (%lu)", query.m().beginTakeReadings.callerTime);

        clock.setTime(query.m().queryCapabilities.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES;
        reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        reply.m().capabilities.type = (fk_module_ModuleType)info->type;
        reply.m().capabilities.name.arg = (void *)info->name;
        reply.m().capabilities.module.arg = (void *)info->module;
        reply.m().capabilities.numberOfSensors = info->numberOfSensors;
        reply.m().capabilities.minimumNumberOfReadings = info->minimumNumberOfReadings;
        reply.m().capabilities.firmware.git.funcs.encode = pb_encode_string;
        reply.m().capabilities.firmware.git.arg = (void *)firmware_version_get();
        reply.m().capabilities.firmware.build.funcs.encode = pb_encode_string;
        reply.m().capabilities.firmware.build.arg = (void *)firmware_build_get();
        reply.m().capabilities.compiled = firmware_compiled_get();
        reply.m().capabilities.requiredUptime = (fk_module_RequiredUptime)info->uptime;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_SENSOR_CAPABILITIES: {
        auto index = query.m().querySensorCapabilities.sensor;

        log("Sensor #%lu: info", index);

        SensorInfo &sensor = info->sensors[index];

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_SENSOR_CAPABILITIES;
        reply.m().sensorCapabilities.id = index;
        reply.m().sensorCapabilities.name.arg = (void *)sensor.name;
        reply.m().sensorCapabilities.unitOfMeasure.arg = (void *)sensor.unitOfMeasure;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS: {
        log("Begin readings (%lu) (%lu)",
            query.m().beginTakeReadings.callerTime,
            query.m().beginTakeReadings.number
        );

        clock.setTime(query.m().beginTakeReadings.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_BEGIN;
        reply.m().readingStatus.backoff = 1000;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            info->readings[i].status = SensorReadingStatus::Busy;
        }

        pending.number = query.m().beginTakeReadings.number;
        pending.elapsed = 0;
        pending.readings = info->readings;

        services().readings->begin(pending.number);

        transit(services().callbacks->states().readings);

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_READING_STATUS: {
        log("Reading status (%lu)", query.m().queryReadingStatus.sleep);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_IDLE;
        reply.m().readingStatus.backoff = 1000;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            if (info->readings[i].status == SensorReadingStatus::Busy) {
                if (reply.m().readingStatus.state == fk_module_ReadingState_BUSY) {
                    reply.m().readingStatus.state = fk_module_ReadingState_BUSY;
                }
            }
            if (info->readings[i].status == SensorReadingStatus::Done) {
                trace("Sending reading %d", i);
                reply.m().readingStatus.state = fk_module_ReadingState_DONE;
                reply.m().readingStatus.elapsed = pending.elapsed;
                reply.m().sensorReading.sensor = i;
                reply.m().sensorReading.time = info->readings[i].time;
                reply.m().sensorReading.value = info->readings[i].value;
                info->readings[i].status = SensorReadingStatus::Idle;
                break;
            }
        }

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_CUSTOM: {
        log("Custom message");

        if (services().callbacks->states().message) {
            transit(services().callbacks->states().message);
        }
        else {
            ModuleReplyMessage reply(*pool);
            reply.m().type = fk_module_ReplyType_REPLY_ERROR;
            outgoing.write(reply);
        }

        break;
    }
    case fk_module_QueryType_QUERY_DATA_PREPARE: {
        log("DataPrepare (size=%lu kind=%lu bank=%lu etag='%s')",
            query.m().data.size, query.m().data.kind,
            query.m().data.bank, (const char *)query.m().data.etag.arg);

        auto enableSpi = services().hardware->enable_spi();
        auto failed = false;

        log("Reclaim...");
        if (!services().flashFs->reclaim(*services().flashState)) {
            log("Flash reclaim failed.");
            failed = true;
        }
        else {
            log("Preallocate...");
            if (!services().flashFs->preallocate()) {
                log("Preallocate failed.");
                failed = true;
            }
        }

        ModuleReplyMessage reply(*pool);

        if (failed) {
            reply.m().type = fk_module_ReplyType_REPLY_ERROR;
        }
        else {
            reply.m().type = fk_module_ReplyType_REPLY_DATA;

            ModuleCopySettings settings{
                (FirmwareBank)query.m().data.bank,
                (uint32_t)query.m().data.size,
                // NOTE: This is freed on the following transition.
                // (const char *)query.m().data.etag.arg
                nullptr
            };

            transit_into<ModuleReceiveData>(settings);
        }

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_DATA_VERIFY: {
        auto checksumData = (pb_data_t *)query.m().data.checksum.arg;
        auto enableSpi = services().hardware->enable_spi();
        auto bank = (FirmwareBank)query.m().data.bank;
        auto expectedSize = query.m().data.size;

        uint32_t expectedChecksum;
        memcpy(&expectedChecksum, checksumData->buffer, sizeof(uint32_t));

        log("DataVerify (size=%lu kind=%lu bank=%lu) (checksum=0x%lx)",
            query.m().data.size, query.m().data.kind, query.m().data.bank, expectedChecksum);

        ModuleReplyMessage reply(*pool);

        FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

        auto address = services().dataCopyStatus.pending;
        auto actualChecksum = services().dataCopyStatus.checksum;

        if (actualChecksum == expectedChecksum) {
            if (!firmwareStorage.verify(address, expectedSize)) {
                error("Error verifying bank!");
                reply.m().type = fk_module_ReplyType_REPLY_ERROR;
            }
            else {
                if (!firmwareStorage.update(bank, address)) {
                    error("Error updating bank!");
                    reply.m().type = fk_module_ReplyType_REPLY_ERROR;
                }
                else {
                    log("Verified! Saved!");
                    reply.m().type = fk_module_ReplyType_REPLY_SUCCESS;

                    transit<ModuleFirmwareSelfFlash>();
                }
            }
        }
        else {
            error("Checksum mismatch!");
            reply.m().type = fk_module_ReplyType_REPLY_ERROR;
        }

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_DATA_APPEND: {
        log("DataAppend (%lu)", query.m().data.size);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_DATA;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_DATA_CLEAR: {
        log("Data clear");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_DATA;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_FIRMWARE: {
        log("Firmware query");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_FIRMWARE;

        auto enableSpi = services().hardware->enable_spi();

        FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };
        firmware_header_t header;

        if (firmwareStorage.header(FirmwareBank::Pending, header)) {
            reply.m().firmware.pending.etag.arg = (void *)pool->strdup(header.etag);
            reply.m().firmware.pending.size = header.size;
        }

        if (firmwareStorage.header(FirmwareBank::Safe, header)) {
            reply.m().firmware.good.etag.arg = (void *)pool->strdup(header.etag);
            reply.m().firmware.good.size = header.size;
        }

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TRANSMISSION: {
        log("Begin transmission");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_TRANSMISSION_STATUS;
        reply.m().transmissionStatus.state = fk_module_TransmissionState_TRANSMISSION_IDLE;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_TRANSMISSION_STATUS: {
        log("Transmission status");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_TRANSMISSION_STATUS;
        reply.m().transmissionStatus.state = fk_module_TransmissionState_TRANSMISSION_IDLE;

        outgoing.write(reply);

        break;
    }
    default: {
        log("Unknown query: %d", query.m().type);
        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_ERROR;
        if (!outgoing.write(reply)) {
            log("Error writing reply");
        }
        break;
    }
    }
}

}
