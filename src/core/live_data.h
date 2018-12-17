#ifndef FK_LIVE_DATA_H_INCLUDED
#define FK_LIVE_DATA_H_INCLUDED

namespace fk {

class LiveDataManager {
private:
    uint32_t enabled_{ 0 };
    uint32_t previously_read_{ 0 };
    uint32_t interval_{ 0 };
    uint32_t reading_{ 0 };

public:
    void configure(uint32_t interval) {
        if (enabled_ == 0) {
            enabled_ = fk_uptime();
        }
        previously_read_ = fk_uptime();
        interval_ = interval;
        completed();
    }

    void completed() {
        if (interval_ == 0) {
            reading_ = 0;
            enabled_ = 0;
            previously_read_ = 0;
        }
        else {
            reading_ = fk_uptime() + interval_;
        }
    }

    bool is_ready_for_reading() {
        if (fk_uptime() - previously_read_ > LivePollInactivity) {
            configure(0);
            return false;
        }

        if (interval_ > 0) {
            if (fk_uptime() > reading_) {
                reading_ = 0;
                return true;
            }
        }

        return false;
    }

};

}

#endif
