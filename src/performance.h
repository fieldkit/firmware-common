#ifndef FK_PERFORMANCE_H_INCLUDED
#define FK_PERFORMANCE_H_INCLUDED

#include <cinttypes>
#include "platform.h"

namespace fk {

class Timer {
private:
    uint32_t total_{ 0 };
    uint32_t started_{ 0 };
    uint32_t counter_{ 0 };

public:
    uint32_t total() {
        return total_;
    }

public:
    struct Started {
    private:
        Timer *owner;

    public:
        Started(Timer *owner) : owner(owner) {
        }

        ~Started() {
            owner->stop();
        }
    };

public:
    Started start() {
        started_ = fk_uptime();
        return { this };
    }

    void stop() {
        total_ += fk_uptime() - started_;
        counter_++;
    }

    void reset() {
        total_ = counter_ = started_ = 0;
    }

};

class Performance {
public:
    static Timer TwoWireSend;
    static Timer TwoWireReceive;
    static Timer Alive;
    static Timer Working;
    static Timer Copying;
    static Timer Idle;

public:
    static void log();

public:
    static void reset() {
        TwoWireSend.reset();
        TwoWireReceive.reset();
        Alive.reset();
        Working.reset();
        Copying.reset();
        Idle.reset();
    }

};

#ifdef FK_ENABLE_PERF
#define FK_PERF_ACQUIRE(obj) auto timer = Performance::obj.start()
#else
#define FK_PERF_ACQUIRE(obj)
#endif

}

#endif
