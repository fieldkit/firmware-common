#ifndef FK_TRANSMIT_READINGS_H_INCLUDED
#define FK_TRANSMIT_READINGS_H_INCLUDED

#include "active_object.h"
#include "pool.h"
#include "wifi.h"
#include "http_post.h"
#include "core_state.h"
#include "fkfs_tasks.h"

namespace fk {

class TransmitAllQueuedReadings : public ActiveObject {
private:
    FkfsIterator iterator;
    CoreState *state;
    Wifi *wifi;
    HttpTransmissionConfig *config;
    Pool *pool;
    bool connected{ false };
    WiFiClient wcl;
    HttpResponseParser parser;

public:
    TransmitAllQueuedReadings(fkfs_t &fs, uint8_t file, CoreState &state, Wifi &wifi, HttpTransmissionConfig &config, Pool &pool);

public:
    void enqueued();
    TaskEval task() override;

private:
    TaskEval openConnection();
    void parseRecord(DataBlock &data);

};

}

#endif
