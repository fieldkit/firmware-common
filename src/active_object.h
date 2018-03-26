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
    Busy,
    Done,
    Error,
};

class TaskEval {
private:
    TaskEval() {
    }

    TaskEval(TaskEvalState state) : state(state) {
    }

public:
    TaskEvalState state{ TaskEvalState::Busy };

    bool isBusy() {
        return state == TaskEvalState::Busy;
    }

    bool isDone() {
        return state == TaskEvalState::Done;
    }

    bool isError() {
        return state == TaskEvalState::Error;
    }

    static TaskEval idle() {
        return TaskEval{ TaskEvalState::Busy };
    }

    static TaskEval busy() {
        return TaskEval{ TaskEvalState::Busy };
    }

    static TaskEval done() {
        return TaskEval{ TaskEvalState::Done };
    }

    static TaskEval error() {
        return TaskEval{ TaskEvalState::Error };
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
class TaskCollection : public Task {
private:
    bool sequential { false };
    std::array<Task*, Size> tasks;

public:
    TaskCollection(bool sequential, std::array<Task*, Size>&& array) : Task("Tasks"), sequential(sequential), tasks(array) {
    }

public:
    TaskEval task() override {
        auto e = TaskEval::done();

        for (std::size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                auto taskStatus = tasks[i]->task();
                if (!taskStatus.isDone()) {
                    e = TaskEval::busy();
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

class TaskQueue {
public:
    virtual bool push(Task &task) = 0;
};

template<std::size_t Size>
class Supervisor : public TaskQueue {
private:
    size_t size{ Size };
    std::array<Task*, Size> tasks;

public:
    Supervisor() {
        for (size_t i = 0; i < Size; ++i) {
            tasks[i] = nullptr;
        }
    }

    bool tick() {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                auto status = tasks[i]->task();
                switch (status.state) {
                case TaskEvalState::Busy: {
                    break;
                }
                case TaskEvalState::Done: {
                    debugfpln("Supervisor", "Done %s", tasks[i]->name);
                    tasks[i]->done();
                    tasks[i] = nullptr;
                    break;
                }
                case TaskEvalState::Error: {
                    debugfpln("Supervisor", "Error %s", tasks[i]->name);
                    tasks[i]->error();
                    tasks[i] = nullptr;
                    break;
                }
                }
            }
        }
        return true;
    }

    bool push(Task &task) override {
        shrink();

        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] == nullptr) {
                debugfpln("Supervisor", "Queuing %s", task.name);
                task.enqueued();
                tasks[i] = &task;
                return true;
            }
        }

        debugfpln("Supervisor", "Unable to queue %s, full!", task.name);

        return false;
    }

private:
    void shrink() {
        for (size_t i = 0; i < Size; ++i) {

        }
    }

};

}

#endif
