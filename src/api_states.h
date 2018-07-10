#ifndef FK_API_STATES_H_INCLUDED
#define FK_API_STATES_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiQueryModule : public WifiState {
private:

public:
    WifiQueryModule() {
    }

public:
    const char *name() const override {
        return "WifiQueryModule";
    }

public:
    void task() override {
        transit<WifiConnectionCompleted>();
    }

};

class WifiDownloadFile : public WifiState {
private:
    FileCopySettings settings_{ FileNumber::StartupLog };
    WifiConnection *connection_{ nullptr };

public:
    const char *name() const override {
        return "WifiDownloadFile";
    }

public:
    WifiDownloadFile() {
    }

    WifiDownloadFile(FileCopySettings settings, WifiConnection &connection) : settings_(settings), connection_(&connection) {
    }

public:
    void task() override {
        StaticPool<384> pool{"WifiDownloadFile"};
        AppReplyMessage reply(&pool);

        DownloadFileTask task{
            *services().fileSystem,
            *services().state,
            reply,
            connection_->getBuffer(),
            *connection_,
            settings_
        };

        task.enqueued();

        while (simple_task_run(task)) {
            services().leds->task();
            services().watchdog->task();
        }

        services().appServicer->flushAndClose();

        transit<WifiConnectionCompleted>();
    }
};

class WifiLiveData : public WifiState {
private:
    uint32_t interval_{ 0 };
    uint32_t lastReadings_{ 0 };
    uint32_t lastPolled_{ 0 };

public:
    WifiLiveData() {
    }

    WifiLiveData(uint32_t interval) : interval_(interval) {
    }

public:
    const char *name() const override {
        return "WifiLiveData";
    }

public:
    void react(LiveDataEvent const &lde) override {
        interval_ = lde.interval;
    }

    void react(AppQueryEvent const &aqe) override {
        if (aqe.type == fk_app_QueryType_QUERY_LIVE_DATA_POLL) {
            lastPolled_ = fk_uptime();
        }
    }

    void entry() override {
        WifiState::entry();

        lastReadings_ = 0;

        if (services().state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
            log("No attached modules.");
            transit<WifiConnectionCompleted>();
            return;
        }
    }

    void task() override {
        if (interval_ == 0) {
            log("Cancelled");
            back();
            return;
        }

        if (fk_uptime() - lastPolled_ > LivePollInactivity) {
            log("Stopped due to inactivity.");
            transit<WifiConnectionCompleted>();
            return;
        }

        if (fk_uptime() - lastReadings_ > interval_) {
            log("Readings");
            lastReadings_ = fk_uptime();
        }

        serve();
    }
};

}

#endif
