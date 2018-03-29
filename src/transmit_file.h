#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "wifi.h"
#include "core_state.h"
#include "fkfs_tasks.h"
#include "utils.h"
#include "file_system.h"

namespace fk {

class TransmitFileTask : public Task {
private:
    FkfsIterator iterator;
    CoreState *state;
    Wifi *wifi;
    HttpTransmissionConfig *config;
    bool connected{ false };
    WiFiClient wcl;
    HttpResponseParser parser;
    CachedDnsResolution cachedDns;

public:
    TransmitFileTask(FileSystem &fileSystem, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config);

public:
    void enqueued();
    TaskEval task() override;

private:
    TaskEval openConnection();

};

class TransmitAllFilesTask : public Task {
private:
    TaskQueue *taskQueue;
    SimpleQueue<TransmitFileTask, 2, FileSystem&, uint8_t, CoreState&, Wifi&, HttpTransmissionConfig&> queue;

public:
    TransmitAllFilesTask(TaskQueue &taskQueue, FileSystem &fileSystem, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config);

public:
    TaskEval task() override;

};

}

#endif
