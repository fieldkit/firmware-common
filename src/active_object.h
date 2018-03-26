#ifndef FK_ACTIVE_OBJECT_H_INCLUDED
#define FK_ACTIVE_OBJECT_H_INCLUDED

#include <cstdarg>
#include <cstdint>

#undef min
#undef max
#include <array>

#include <Arduino.h>

#include "debug.h"

namespace fk {

class Task;

enum class TaskEvalState {
    Idle,
    Yield,
    Done,
    Error,
};

class TaskEval {
private:
    TaskEval() {
    }

    TaskEval(TaskEvalState state) : state(state) {
    }

    TaskEval(TaskEvalState state, Task &task) : state(state), nextTask(&task) {
    }

public:
    TaskEvalState state{ TaskEvalState::Idle };
    Task *nextTask{ nullptr };

    bool isIdle() {
        return state == TaskEvalState::Idle;
    }

    bool isYield() {
        return state == TaskEvalState::Yield;
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

    static TaskEval yield() {
        return TaskEval{ TaskEvalState::Yield };
    }

    static TaskEval yield(Task &task) {
        return TaskEval{ TaskEvalState::Yield, task };
    }

    static TaskEval done() {
        return TaskEval{ TaskEvalState::Done };
    }

    static TaskEval error() {
        return TaskEval{ TaskEvalState::Error };
    }

    static TaskEval pass(Task &task) {
        return TaskEval{ TaskEvalState::Done, task };
    }

};

class Task {
public:
    const char *name{ nullptr };
    Task *nextTask{ nullptr };

public:
    Task(const char *name) : name(name) {}

public:
    virtual void enqueued() {}
    virtual void done() {}
    virtual void error() {}
    virtual TaskEval task() = 0;

public:
    const char *toString() const {
        return name;
    }

    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));

};

inline bool areSame(const Task &a, const Task &b) {
    return &a == &b;
}

class ActiveObject : public Task {
private:
    Task *idleTask{ nullptr };
    Task *tasks{ nullptr };

public:
    ActiveObject(const char *name);
    ActiveObject(const char *name, Task &idleTask);

public:
    TaskEval task() override;

public:
    bool inQueue(Task &task);
    void push(Task &task);
    void tick();
    bool isIdle();
    void cancel();

public:
    virtual void done(Task &task);
    virtual void error(Task &task);
    virtual void idle();

protected:
    void service(Task &task);

private:
    void pop();
    Task *tail();
    Task **end();

};

class Delay : public Task {
    uint32_t duration{ 0 };
    uint32_t dieAt{ 0 };
    bool startOnEnqueue{ true };

public:
    Delay(uint32_t duration, bool startOnEnqueue = true) : Task("Delay"), duration(duration), dieAt(millis() + duration), startOnEnqueue(startOnEnqueue) {
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
            dieAt = millis() + duration;
        }
        if (millis() > dieAt) {
            return TaskEval::done();
        }
        return TaskEval::idle();
    }

};

template<std::size_t Size>
class TaskCollection {
private:
    bool sequential { false };
    std::array<Task*, Size> tasks;

public:
    TaskCollection(bool sequential, std::array<Task*, Size>&& array) : sequential(sequential), tasks(array) {
    }

public:
    TaskEval work() {
        auto e = TaskEval::done();

        for (std::size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                auto taskStatus = tasks[i]->work();
                if (!taskStatus.isDone()) {
                    e = TaskEval::idle();
                    if (sequential) {
                        break;
                    }
                } else {
                    tasks[i] = nullptr;
                }
            }
        }

        return e;
    }

};

template<typename ...T>
auto to_parallel_task_collection(T&&... tasks) -> TaskCollection<sizeof...(T)> {
    return TaskCollection<sizeof...(T)>{ false, { { std::forward<T>(tasks)... } } };
}

template<typename ...T>
auto to_sequential_task_collection(T&&... tasks) -> TaskCollection<sizeof...(T)> {
    return TaskCollection<sizeof...(T)>{ true, { { std::forward<T>(tasks)... } } };
}

}

#endif
