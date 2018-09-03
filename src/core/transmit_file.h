#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "task.h"
#include "wifi.h"
#include "core_state.h"
#include "file_reader.h"
#include "wifi_tools.h"
#include "file_system.h"
#include "url_parser.h"
#include "http_response_parser.h"

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

}

#endif
