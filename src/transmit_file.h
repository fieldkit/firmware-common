#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "wifi.h"
#include "core_state.h"
#include "file_reader.h"
#include "utils.h"
#include "file_system.h"

namespace fk {

class TransmitFileTask : public Task {
private:
    FileSystem *fileSystem;
    CoreState *state;
    Wifi *wifi;
    HttpTransmissionConfig *config;
    uint32_t waitingSince{ 0 };
    bool connected{ false };
    uint8_t tries{ 0 };
    WiFiClient wcl;
    HttpResponseParser parser;
    CachedDnsResolution cachedDns;

public:
    TransmitFileTask(FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config);

public:
    void enqueued();
    TaskEval task() override;

private:
    TaskEval openConnection();

};

class FileCopierSample : public Task {
private:
    FileSystem *fileSystem;
    CoreState *state;

public:
    FileCopierSample(FileSystem &fileSystem, CoreState &state);

public:
    void enqueued();
    TaskEval task() override;

};

}

#endif
