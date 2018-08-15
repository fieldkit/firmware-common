#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "task.h"
#include "pool.h"
#include "wifi.h"
#include "core_state.h"
#include "file_reader.h"
#include "utils.h"
#include "file_system.h"

namespace fk {

class TransmitFileTask : public Task, public FileCopyCallbacks {
private:
    FileSystem *fileSystem;
    CoreState *state;
    Wifi *wifi;
    HttpTransmissionConfig *config;
    FileCopySettings settings;
    WiFiClient wcl;
    HttpResponseParser parser;
    CachedDnsResolution cachedDns;
    uint32_t copyFinishedAt{ 0 };
    bool connected{ false };
    uint8_t tries{ 0 };

public:
    TransmitFileTask(FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config, FileCopySettings settings);

public:
    void enqueued();
    void fileCopyTick() override;
    TaskEval task() override;

private:
    bool openFile();
    bool writeBeginning(Url &parsed);
    TaskEval openConnection();

};

class FileCopierSample : public Task {
private:
    FileSystem *fileSystem_;
    CoreState *state_;

public:
    FileCopierSample(FileSystem &fileSystem, CoreState &state);

public:
    void enqueued();
    TaskEval task() override;

};

}

#endif
