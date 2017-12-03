#ifndef FK_ACTIVE_OBJECT_H_INCLUDED
#define FK_ACTIVE_OBJECT_H_INCLUDED

#include <cstdint>
#include <cstdarg>

#include <Arduino.h>

#include "debug.h"

namespace fk {

class Task;

class TaskEval {
private:
    TaskEval() {
    }

public:
    Task *task { nullptr };

    static TaskEval Idle;
    static TaskEval Done;
    static TaskEval Error;
    static TaskEval Pass;

    static TaskEval &pass(Task &task) {
        Pass.task = &task;
        return Pass;
    }
};

inline bool areSame(const TaskEval& a, const TaskEval& b) {
    return &a == &b;
}

class Task {
public:
    const char *name { nullptr };
    Task *nextTask { nullptr };

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

    virtual TaskEval& task() = 0;

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

inline bool areSame(const Task& a, const Task& b) {
    return &a == &b;
}

class Delay : public Task  {
    uint32_t duration { 0 };
    uint32_t dieAt { 0 };

public:
    Delay(uint32_t duration) : Task("Delay"), duration(duration) {
    }

    void enqueued() override {
        dieAt = 0;
    }

    TaskEval &task() override {
        if (dieAt == 0){
            dieAt = millis() + duration;
        }
        if (millis() > dieAt) {
            return TaskEval::Done;
        }
        return TaskEval::Idle;
    }

};

class ActiveObject {
private:
    Task *idleTask { nullptr };
    Task *tasks { nullptr };

public:
    ActiveObject();
    ActiveObject(Task &idleTask);

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
