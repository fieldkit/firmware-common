#include <functional>

#include <FuelGauge.h>

#include "app_servicer.h"
#include "utils.h"
#include "device_id.h"

#include "download_file_task.h"
#include "leds.h"
#include "watchdog.h"
#include "api_states.h"

namespace fk {

static void copy(ScheduledTask &to, fk_app_Schedule &from);

static void copy(fk_app_Schedule &to, ScheduledTask &from);

AppServicer::AppServicer(CoreState &state, Scheduler &scheduler, FkfsReplies &fileReplies, WifiConnection &connection, ModuleCommunications &communications, Pool &pool)
    : ApiConnection(connection, pool), state_(&state), scheduler_(&scheduler), fileReplies_(&fileReplies), communications_(&communications) {
}

void AppServicer::react(LiveDataEvent const &lde) {
    transit_into<WifiLiveData>(lde.interval);
}

bool AppServicer::handle() {
    switch (query_.m().type) {
    case fk_app_QueryType_QUERY_CAPABILITIES: {
        capabilitiesReply();

        break;
    }
    case fk_app_QueryType_QUERY_LIVE_DATA_POLL: {
        log("Live ds (interval = %lu)", query_.m().liveDataPoll.interval);

        send_event(LiveDataEvent{
            query_.m().liveDataPoll.interval
        });

        auto numberOfReadings = state_->numberOfReadings();
        fk_app_LiveDataSample samples[numberOfReadings];

        for (size_t i = 0; i < numberOfReadings; ++i) {
            auto available = state_->getReading(i);
            samples[i].sensor = available.id;
            samples[i].time = available.reading.time;
            samples[i].value = available.reading.value;
        }

        state_->clearReadings();

        pb_array_t live_data_array = {
            .length = numberOfReadings,
            .itemSize = sizeof(fk_app_LiveDataSample),
            .buffer = samples,
            .fields = fk_app_LiveDataSample_fields,
        };

        reply_.m().type = fk_app_ReplyType_REPLY_LIVE_DATA_POLL;
        reply_.m().liveData.samples.funcs.encode = pb_encode_array;
        reply_.m().liveData.samples.arg = (void *)&live_data_array;

        if (!buffer().write(reply_)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_CONFIGUE_SCHEDULES: {
        log("Configure schedules");

        auto &readings = scheduler_->getTaskSchedule(ScheduleKind::Readings);
        auto &transmission = scheduler_->getTaskSchedule(ScheduleKind::Transmission);
        auto &status = scheduler_->getTaskSchedule(ScheduleKind::Status);
        auto &location = scheduler_->getTaskSchedule(ScheduleKind::Location);

        reply_.m().type = fk_app_ReplyType_REPLY_SCHEDULES;
        copy(readings, reply_.m().schedules.readings);
        copy(transmission, reply_.m().schedules.transmission);
        copy(status, reply_.m().schedules.status);
        copy(location, reply_.m().schedules.location);
        copy(reply_.m().schedules.readings, readings);
        copy(reply_.m().schedules.transmission, transmission);
        copy(reply_.m().schedules.status, status);
        copy(reply_.m().schedules.location, location);

        if (!buffer().write(reply_)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_SCHEDULES: {
        log("Query schedules");

        auto &readings = scheduler_->getTaskSchedule(ScheduleKind::Readings);
        auto &transmission = scheduler_->getTaskSchedule(ScheduleKind::Transmission);
        auto &status = scheduler_->getTaskSchedule(ScheduleKind::Status);
        auto &location = scheduler_->getTaskSchedule(ScheduleKind::Location);

        reply_.m().type = fk_app_ReplyType_REPLY_SCHEDULES;
        copy(reply_.m().schedules.readings, readings);
        copy(reply_.m().schedules.transmission, transmission);
        copy(reply_.m().schedules.status, status);
        copy(reply_.m().schedules.location, location);

        if (!buffer().write(reply_)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_RESET: {
        log("Reset");

        fileReplies_->resetAll(*state_);

        reply_.m().type = fk_app_ReplyType_REPLY_SUCCESS;

        if (!buffer().write(reply_)) {
            log("Error writing reply");
        }

        break;
    }
    case fk_app_QueryType_QUERY_FILES: {
        log("Query files");

        fileReplies_->queryFilesReply(query_, reply_, buffer());

        break;
    }
    case fk_app_QueryType_QUERY_DOWNLOAD_FILE: {
        transit_into<WifiDownloadFile>(FileCopySettings{
                (FileNumber)query_.m().downloadFile.id,
                query_.m().downloadFile.offset,
                query_.m().downloadFile.length,
            }, std::ref(connection()));
        return false;
    }
    case fk_app_QueryType_QUERY_ERASE_FILE: {
        log("Erase file (%lu)", query_.m().eraseFile.id);

        fileReplies_->eraseFileReply(query_, reply_, buffer());

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
        // auto address = (uint8_t)query.m().module.address;
        transit_into<WifiQueryModule>();
        return false;
    }
    case fk_app_QueryType_QUERY_CONFIGURE_SENSOR:
    default: {
        reply_.error("Unknown query");
        if (!buffer().write(reply_)) {
            log("Error writing reply");
        }

        break;
    }
    }

    return true;
}

void AppServicer::capabilitiesReply() {
    log("Query caps");

    auto *attached = state_->attachedModules();
    auto numberOfModules = state_->numberOfModules();
    auto numberOfSensors = state_->numberOfSensors();
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

    reply_.m().type = fk_app_ReplyType_REPLY_CAPABILITIES;
    reply_.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
    reply_.m().capabilities.name.funcs.encode = pb_encode_string;
    reply_.m().capabilities.name.arg = (void *)DefaultName;
    reply_.m().capabilities.sensors.funcs.encode = pb_encode_array;
    reply_.m().capabilities.sensors.arg = (void *)&sensorsArray;
    reply_.m().capabilities.modules.funcs.encode = pb_encode_array;
    reply_.m().capabilities.modules.arg = (void *)&modulesArray;
    reply_.m().capabilities.deviceId.funcs.encode = pb_encode_data;
    reply_.m().capabilities.deviceId.arg = &deviceIdData;

    if (!buffer().write(reply_)) {
        log("Error writing reply");
    }
}

void AppServicer::configureNetworkSettings() {
    log("Configure network settings...");

    auto networksArray = (pb_array_t *)query_.m().networkSettings.networks.arg;
    auto newNetworks = (fk_app_NetworkInfo *)networksArray->buffer;

    log("Networks: %d", networksArray->length);

    auto settings = state_->getNetworkSettings();
    settings.createAccessPoint = query_.m().networkSettings.createAccessPoint;
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

    state_->configure(settings);
}

void AppServicer::networkSettingsReply() {
    log("Network settings");

    auto currentSettings = state_->getNetworkSettings();
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

    reply_.m().type = fk_app_ReplyType_REPLY_NETWORK_SETTINGS;
    reply_.m().networkSettings.createAccessPoint = currentSettings.createAccessPoint;
    reply_.m().networkSettings.networks.arg = &networksArray;
    reply_.m().networkSettings.networks.funcs.encode = pb_encode_array;
    if (!buffer().write(reply_)) {
        log("Error writing reply");
    }
}

void AppServicer::statusReply() {
    log("Status");

    FuelGauge fuelGage;
    reply_.m().type = fk_app_ReplyType_REPLY_STATUS;
    reply_.m().status.uptime = fk_uptime();
    reply_.m().status.batteryPercentage = fuelGage.stateOfCharge();
    reply_.m().status.batteryVoltage = fuelGage.cellVoltage();
    reply_.m().status.gpsHasFix = false;
    reply_.m().status.gpsSatellites = 0;
    if (!buffer().write(reply_)) {
        log("Error writing reply");
    }
}

void AppServicer::configureIdentity() {
    DeviceIdentity identity{
        (const char *)query_.m().identity.device.arg,
        (const char *)query_.m().identity.stream.arg,
    };

    state_->configure(identity);
}

void AppServicer::identityReply() {
    log("Identity");

    pb_data_t deviceIdData = {
        .length = deviceId.length(),
        .buffer = deviceId.toBuffer(),
    };

    auto identity = state_->getIdentity();
    reply_.m().type = fk_app_ReplyType_REPLY_IDENTITY;
    reply_.m().identity.device.arg = identity.device;
    reply_.m().identity.stream.arg = identity.stream;
    reply_.m().identity.deviceId.funcs.encode = pb_encode_data;
    reply_.m().identity.deviceId.arg = &deviceIdData;
    if (!buffer().write(reply_)) {
        log("Error writing reply");
    }
}

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

}
