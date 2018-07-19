#ifndef FK_LIVE_DATA_H_INCLUDED
#define FK_LIVE_DATA_H_INCLUDED

namespace fk {

class LiveDataManager {
private:
    uint32_t enabled_{ 0 };
    uint32_t interval_{ 0 };
    uint32_t takeReadingAt_{ 0 };

public:
    LiveDataManager() {
    }

public:
    void configure(uint32_t interval) {
        enabled_= fk_uptime();
        interval_ = interval;
        completed();
    }

public:
    void completed() {
        if (interval_ == 0) {
            takeReadingAt_ = 0;
        }
        else {
            takeReadingAt_ = fk_uptime() + interval_;
        }
    }

    bool takeReading() {
        if (interval_ > 0) {
            if (fk_uptime() > takeReadingAt_) {
                takeReadingAt_ = 0;
                return true;
            }
        }
        return false;
    }

};

}

#endif
