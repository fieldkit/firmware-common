#ifndef FK_DELAY_H_INCLUDED
#define FK_DELAY_H_INCLUDED

namespace fk {

class Delay : public Task {
    uint32_t duration{ 0 };
    uint32_t dieAt{ 0 };
    bool startOnEnqueue{ true };

public:
    Delay(uint32_t duration, bool startOnEnqueue = true) :
        Task("Delay"), duration(duration), dieAt(fk_uptime() + duration), startOnEnqueue(startOnEnqueue) {
    }

    void adjust(uint32_t d) {
        duration = d;
        dieAt = 0;
    }

    void enqueued() override {
        if (startOnEnqueue) {
            dieAt = 0;
        }
    }

    TaskEval task() override {
        if (dieAt == 0) {
            dieAt = fk_uptime() + duration;
        }
        if (fk_uptime() > dieAt) {
            return TaskEval::done();
        }
        return TaskEval::idle();
    }

};

class NoopTask  : public Task {
public:
    NoopTask() : Task("Noop") {
    }

public:
    TaskEval task() override {
        return TaskEval::done();
    }

};

}

#endif
