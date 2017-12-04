#ifndef FK_ACTIVE_OBJECT_H_INCLUDED
#define FK_ACTIVE_OBJECT_H_INCLUDED

#include <cstdarg>
#include <cstdint>

#include <Arduino.h>

#include "debug.h"

namespace fk {

class Task;

enum class TaskEvalState {
    Idle,
    Done,
    Error
};

class TaskEval {
private:
    TaskEval() {
    }

    TaskEval(TaskEvalState state) : state(state) {
    }

    TaskEval(Task &task) : state(TaskEvalState::Done), task(&task) {
    }

public:
    TaskEvalState state{ TaskEvalState::Idle };
    Task *task{ nullptr };

    bool isIdle() {
        return state == TaskEvalState::Idle;
    }

    bool isDone() {
        return state == TaskEvalState::Done;
    }

    bool isError() {
        return state == TaskEvalState::Error;
    }

    static TaskEval idle() {
        return TaskEval{ TaskEvalState::Idle };
    }

    static TaskEval done() {
        return TaskEval{ TaskEvalState::Done };
    }

    static TaskEval error() {
        return TaskEval{ TaskEvalState::Error };
    }

    static TaskEval pass(Task &task) {
        return TaskEval{ task };
    }
};

class Task {
public:
    const char *name{ nullptr };
    Task *nextTask{ nullptr };

public:
    Task(const char *name) : name(name) {
    }

public:
    virtual void enqueued() {
    }

    virtual void done() {
    }

    virtual void error() {
    }

    virtual TaskEval task() = 0;

public:
    const char *toString() const {
        return name;
    }

    void log(const char *f, ...) const {
        va_list args;
        va_start(args, f);
        vdebugfpln(name, f, args);
        va_end(args);
    }

};

inline bool areSame(const Task &a, const Task &b) {
    return &a == &b;
}

class Delay : public Task {
    uint32_t duration{ 0 };
    uint32_t dieAt{ 0 };

public:
    Delay(uint32_t duration) : Task("Delay"), duration(duration) {
    }

    void enqueued() override {
        dieAt = 0;
    }

    TaskEval task() override {
        if (dieAt == 0) {
            dieAt = millis() + duration;
        }
        if (millis() > dieAt) {
            return TaskEval::done();
        }
        return TaskEval::idle();
    }

};

class ActiveObject {
private:
    const char *name{ nullptr };
    Task *idleTask{ nullptr };
    Task *tasks{ nullptr };

public:
    ActiveObject();
    ActiveObject(const char *name);
    ActiveObject(const char *name, Task &idleTask);

public:
    void push(Task &task);
    void tick();
    bool idle();
    void log(const char *f, ...) const;

    virtual void done(Task &task);
    virtual void error(Task &task);

private:
    void pop();
    void service(Task &task);
    Task **end();

};

}

#endif
