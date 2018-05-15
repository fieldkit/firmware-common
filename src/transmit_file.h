#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "wifi.h"
#include "core_state.h"
#include "fkfs_tasks.h"
#include "file_reader.h"
#include "utils.h"
#include "file_system.h"

namespace fk {

class TransmitFileTask : public Task {
private:
    FileReader fileReader;
    CoreState *state;
    Wifi *wifi;
    HttpTransmissionConfig *config;
    uint32_t waitingSince{ 0 };
    bool connected{ false };
    uint8_t tries{ 0 };
    WiFiClient wcl;
    HttpResponseParser parser;
    CachedDnsResolution cachedDns;
    lws::BufferedStreamCopier<256> streamCopier;
    lws::CircularStreams<lws::RingBufferN<256>> outgoing;

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
