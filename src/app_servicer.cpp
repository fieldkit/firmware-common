#include <new>

#include <FuelGauge.h>

#include "app_servicer.h"
#include "utils.h"
#include "device_id.h"

namespace fk {

static void copy(ScheduledTask &to, fk_app_Schedule &from) {
    to.setSecond(TimeSpec{ (int8_t)from.second.fixed, (int8_t)from.second.interval });
    to.setMinute(TimeSpec{ (int8_t)from.minute.fixed, (int8_t)from.minute.interval });
    to.setHour(TimeSpec{ (int8_t)from.hour.fixed, (int8_t)from.hour.interval });
    to.setDay(TimeSpec{ (int8_t)from.day.fixed, (int8_t)from.day.interval });
}

static void copy(fk_app_Schedule &to, ScheduledTask &from) {
    to.second.fixed = from.getSecond().fixed;
    to.second.interval = from.getSecond().interval;
    to.second.offset = 0;
    to.minute.fixed = from.getMinute().fixed;
    to.minute.interval = from.getMinute().interval;
    to.minute.offset = 0;
    to.hour.fixed = from.getHour().fixed;
    to.hour.interval = from.getHour().interval;
    to.hour.offset = 0;
    to.day.fixed = from.getDay().fixed;
    to.day.interval = from.getDay().interval;
    to.day.offset = 0;
}

AppServicer::AppServicer(TwoWireBus &bus, LiveData &liveData, CoreState &state, Scheduler &scheduler, FkfsReplies &fileReplies, WifiConnection &connection, ModuleCommunications &communications, Pool &pool)
    : Task("AppServicer"), bus(&bus), query(&pool), reply(&pool), liveData(&liveData), state(&state), scheduler(&scheduler), fileReplies(&fileReplies), connection(&connection), communications(&communications), pool(&pool) {
}

void AppServicer::enqueued() {
    active.clear();
    bytesRead = 0;
    dieAt = 0;
}

TaskEval AppServicer::task() {
    if (active.hasChild()) {
        return active.task();
    }

    if (!readQuery()) {
        return TaskEval::error();
    }

    if (bytesRead == 0) {
        return TaskEval::idle();
    }

    return handle();
}

bool AppServicer::readQuery() {
    if (dieAt == 0) {
        dieAt = millis() + WifiConnectionTimeout;
    }
    else if (millis() > dieAt) {
        connection->close();
        log("Connection timed out.");
        return false;
    }

    auto read = connection->read();
    if (read > 0) {
        log("Read %d bytes", read);
        bytesRead += read;
        buffer = &connection->getBuffer();
        active.clear();
        if (!buffer->read(query)) {
            connection->close();
            log("Error parsing query");
            return false;
        }
    }

    return true;
}

void AppServicer::done() {
    flushAndClose();
}

void AppServicer::error() {
    flushAndClose();
}

bool AppServicer::flushAndClose() {
    connection->flush();

    if (connection->isConnected()) {
        log("Stop connection");
        connection->close();
    }
    else {
        log("No connection!");
    }

    return true;
}

TaskEval AppServicer::handle() {
    pool->clear();
    buffer->clear();
    reply.clear();

    switch (query.m().type) {
    case fk_app_QueryType_QUERY_CAPABILITIES: {
        capabilitiesReply();

        break;
    }
    case fk_app_QueryType_QUERY_DATA_SET:
    case fk_app_QueryType_QUERY_DATA_SETS: {
        log("Query ds");

        fileReplies->dataSetsReply(query, reply, *buffer);

        break;
    }
    case fk_app_QueryType_QUERY_DOWNLOAD_DATA_SET: {
        log("Download ds %lu page=%lu", query.m().downloadDataSet.id, query.m().downloadDataSet.page);
        auto newTask = fileReplies->downloadDataSetReply(*state, query, reply, *buffer);
        active.push(*newTask);
        return TaskEval::busy();
    }
    case fk_app_QueryType_QUERY_ERASE_DATA_SET: {
        log("Erase ds");

        fileReplies->eraseDataSetReply(query, reply, *buffer);

        break;
    }
    case fk_app_QueryType_QUERY_LIVE_DATA_POLL: {
        log("Live ds (interval = %lu)", query.m().liveDataPoll.interval);

        if (query.m().liveDataPoll.interval > 0) {
            liveData->start(query.m().liveDataPoll.interval);
        }
        else {
            liveData->stop();
        }

        auto numberOfReadings = state->numberOfReadings();
        fk_app_LiveDataSample samples[numberOfReadings];

        for (size_t i = 0; i < numberOfReadings; ++i) {
            auto available = state->getReading(i);
            samples[i].sensor = available.id;
            samples[i].time = available.reading.time;
            samples[i].value = available.reading.value;
        }

        state->clearReadings();

        pb_array_t live_data_array = {
            .length = numberOfReadings,
            .itemSize = sizeof(fk_app_LiveDataSample),
            .buffer = samples,
            .fields = fk_app_LiveDataSample_fields,
        };

        reply.m().type = fk_app_ReplyType_REPLY_LIVE_DATA_POLL;
        reply.m().liveData.samples.funcs.encode = pb_encode_array;
        reply.m().liveData.samples.arg = (void *)&live_data_array;

        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_CONFIGUE_SCHEDULES: {
        log("Configure schedules");

        auto &readings = scheduler->getTaskSchedule(ScheduleKind::Readings);
        auto &transmission = scheduler->getTaskSchedule(ScheduleKind::Transmission);
        auto &status = scheduler->getTaskSchedule(ScheduleKind::Status);
        auto &location = scheduler->getTaskSchedule(ScheduleKind::Location);

        reply.m().type = fk_app_ReplyType_REPLY_SCHEDULES;
        copy(readings, reply.m().schedules.readings);
        copy(transmission, reply.m().schedules.transmission);
        copy(status, reply.m().schedules.status);
        copy(location, reply.m().schedules.location);
        copy(reply.m().schedules.readings, readings);
        copy(reply.m().schedules.transmission, transmission);
        copy(reply.m().schedules.status, status);
        copy(reply.m().schedules.location, location);

        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_SCHEDULES: {
        log("Query schedules");

        auto &readings = scheduler->getTaskSchedule(ScheduleKind::Readings);
        auto &transmission = scheduler->getTaskSchedule(ScheduleKind::Transmission);
        auto &status = scheduler->getTaskSchedule(ScheduleKind::Status);
        auto &location = scheduler->getTaskSchedule(ScheduleKind::Location);

        reply.m().type = fk_app_ReplyType_REPLY_SCHEDULES;
        copy(reply.m().schedules.readings, readings);
        copy(reply.m().schedules.transmission, transmission);
        copy(reply.m().schedules.status, status);
        copy(reply.m().schedules.location, location);

        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_RESET: {
        log("Reset");

        fileReplies->resetAll();

        reply.m().type = fk_app_ReplyType_REPLY_SUCCESS;

        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_FILES: {
        log("Query files");

        fileReplies->queryFilesReply(query, reply, *buffer);

        break;
    }
    case fk_app_QueryType_QUERY_DOWNLOAD_FILE: {
        if (!state->isReadingInProgress()) {
            log("Download file (%lu / %lu)", query.m().downloadFile.id, query.m().downloadFile.page);
            auto newTask = fileReplies->downloadFileReply(*state, query, reply, *buffer);
            active.push(*newTask);
            return TaskEval::busy();
        }

        reply.busy("Busy");
        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;

    }
    case fk_app_QueryType_QUERY_ERASE_FILE: {
        log("Erase file (%lu)", query.m().eraseFile.id);

        fileReplies->eraseFileReply(query, reply, *buffer);

        break;
    }
    case fk_app_QueryType_QUERY_NETWORK_SETTINGS: {
        networkSettingsReply();

        break;
    }
    case fk_app_QueryType_QUERY_CONFIGURE_NETWORK_SETTINGS: {
        configureNetworkSettings();
        networkSettingsReply();

        break;
    }
    case fk_app_QueryType_QUERY_IDENTITY: {
        identityReply();

        break;
    }
    case fk_app_QueryType_QUERY_CONFIGURE_IDENTITY: {
        configureIdentity();
        identityReply();

        break;
    }
    case fk_app_QueryType_QUERY_STATUS: {
        statusReply();

        break;
    }
    case fk_app_QueryType_QUERY_MODULE: {
        auto task = appModuleQueryTask.ready(*bus, reply, query, *buffer, (uint8_t)query.m().module.address, *communications);
        if (peripherals.twoWire1().tryAcquire(task)) {
            active.push(*task);
            return TaskEval::busy();
        }

        reply.busy("Busy");
        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_CONFIGURE_SENSOR:
    default: {
        reply.error("Unknown query");
        if (!buffer->write(reply)) {
            log("Error writing reply");
        }

        break;
    }
    }

    return TaskEval::done();
}

void AppServicer::capabilitiesReply() {
    log("Query caps");

    auto *attached = state->attachedModules();
    auto numberOfModules = state->numberOfModules();
    auto numberOfSensors = state->numberOfSensors();
    auto sensorIndex = 0;
    fk_app_SensorCapabilities sensors[numberOfSensors];
    fk_app_ModuleCapabilities modules[numberOfModules];
    for (size_t moduleIndex = 0; attached[moduleIndex].address > 0; ++moduleIndex) {
        for (size_t i = 0; i < attached[moduleIndex].numberOfSensors; ++i) {
            sensors[sensorIndex].id = i;
            sensors[sensorIndex].name.funcs.encode = pb_encode_string;
            sensors[sensorIndex].name.arg = (void *)attached[moduleIndex].sensors[i].name;
            sensors[sensorIndex].unitOfMeasure.funcs.encode = pb_encode_string;
            sensors[sensorIndex].unitOfMeasure.arg = (void *)attached[moduleIndex].sensors[i].unitOfMeasure;
            sensors[sensorIndex].frequency = 60;
            sensors[sensorIndex].module = moduleIndex;
            sensorIndex++;
        }
        modules[moduleIndex].id = moduleIndex;
        modules[moduleIndex].name.funcs.encode = pb_encode_string;
        modules[moduleIndex].name.arg = (void *)attached[moduleIndex].name;
    }

    pb_array_t modulesArray = {
        .length = numberOfModules,
        .itemSize = sizeof(fk_app_ModuleCapabilities),
        .buffer = modules,
        .fields = fk_app_ModuleCapabilities_fields,
    };

    pb_array_t sensorsArray = {
        .length = numberOfSensors,
        .itemSize = sizeof(fk_app_SensorCapabilities),
        .buffer = sensors,
        .fields = fk_app_SensorCapabilities_fields,
    };

    pb_data_t deviceIdData = {
        .length = deviceId.length(),
        .buffer = deviceId.toBuffer(),
    };

    reply.m().type = fk_app_ReplyType_REPLY_CAPABILITIES;
    reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
    reply.m().capabilities.name.funcs.encode = pb_encode_string;
    reply.m().capabilities.name.arg = (void *)DefaultName;
    reply.m().capabilities.sensors.funcs.encode = pb_encode_array;
    reply.m().capabilities.sensors.arg = (void *)&sensorsArray;
    reply.m().capabilities.modules.funcs.encode = pb_encode_array;
    reply.m().capabilities.modules.arg = (void *)&modulesArray;
    reply.m().capabilities.deviceId.funcs.encode = pb_encode_data;
    reply.m().capabilities.deviceId.arg = &deviceIdData;

    if (!buffer->write(reply)) {
        log("Error writing reply");
    }
}

void AppServicer::configureNetworkSettings() {
    log("Configure network settings...");

    auto networksArray = (pb_array_t *)query.m().networkSettings.networks.arg;
    auto newNetworks = (fk_app_NetworkInfo *)networksArray->buffer;

    log("Networks: %d", networksArray->length);

    auto settings = state->getNetworkSettings();
    settings.createAccessPoint = query.m().networkSettings.createAccessPoint;
    for (size_t i = 0; i < MaximumRememberedNetworks; ++i) {
        if (i < networksArray->length) {
            settings.networks[i] = NetworkInfo{
                (const char *)newNetworks[i].ssid.arg,
                (const char *)newNetworks[i].password.arg
            };
        }
        else {
            settings.networks[i] = NetworkInfo{};
        }
    }

    state->configure(settings);
}

void AppServicer::networkSettingsReply() {
    log("Network settings");

    auto currentSettings = state->getNetworkSettings();
    fk_app_NetworkInfo networks[MaximumRememberedNetworks];
    for (auto i = 0; i < MaximumRememberedNetworks; ++i) {
        networks[i].ssid.arg = currentSettings.networks[i].ssid;
        networks[i].ssid.funcs.encode = pb_encode_string;
        networks[i].password.arg = currentSettings.networks[i].password;
        networks[i].password.funcs.encode = pb_encode_string;
    }

    pb_array_t networksArray = {
        .length = sizeof(networks) / sizeof(fk_app_NetworkInfo),
        .itemSize = sizeof(fk_app_NetworkInfo),
        .buffer = &networks,
        .fields = fk_app_NetworkInfo_fields,
    };

    reply.m().type = fk_app_ReplyType_REPLY_NETWORK_SETTINGS;
    reply.m().networkSettings.createAccessPoint = currentSettings.createAccessPoint;
    reply.m().networkSettings.networks.arg = &networksArray;
    reply.m().networkSettings.networks.funcs.encode = pb_encode_array;
    if (!buffer->write(reply)) {
        log("Error writing reply");
    }
}

void AppServicer::statusReply() {
    log("Status");

    FuelGauge fuelGage;
    reply.m().type = fk_app_ReplyType_REPLY_STATUS;
    reply.m().status.uptime = millis();
    reply.m().status.batteryPercentage = fuelGage.stateOfCharge();
    reply.m().status.batteryVoltage = fuelGage.cellVoltage();
    reply.m().status.gpsHasFix = false;
    reply.m().status.gpsSatellites = 0;
    if (!buffer->write(reply)) {
        log("Error writing reply");
    }
}

void AppServicer::configureIdentity() {
    DeviceIdentity identity{
        (const char *)query.m().identity.device.arg,
        (const char *)query.m().identity.stream.arg,
    };

    state->configure(identity);
}

void AppServicer::identityReply() {
    log("Identity");

    pb_data_t deviceIdData = {
        .length = deviceId.length(),
        .buffer = deviceId.toBuffer(),
    };

    auto identity = state->getIdentity();
    reply.m().type = fk_app_ReplyType_REPLY_IDENTITY;
    reply.m().identity.device.arg = identity.device;
    reply.m().identity.stream.arg = identity.stream;
    reply.m().identity.deviceId.funcs.encode = pb_encode_data;
    reply.m().identity.deviceId.arg = &deviceIdData;
    if (!buffer->write(reply)) {
        log("Error writing reply");
    }
}

}
