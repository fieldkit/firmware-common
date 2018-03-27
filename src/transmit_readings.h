#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "wifi.h"
#include "core_state.h"
#include "fkfs_tasks.h"
#include "utils.h"

namespace fk {

class TransmitAllQueuedReadings : public Task {
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
    TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config);

public:
    void enqueued();
    TaskEval task() override;

private:
    TaskEval openConnection();
    void parseRecord(DataBlock &data);

};

}

#endif
