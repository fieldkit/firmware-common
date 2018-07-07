#ifndef FK_TASK_H_INCLUDED
#define FK_TASK_H_INCLUDED

namespace fk {

class Task;

enum class TaskEvalState {
    Idle,
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

public:
    bool isIdle() {
        return state == TaskEvalState::Idle;
    }

    bool isBusy() {
        return state == TaskEvalState::Busy;
    }

    bool isDone() {
        return state == TaskEvalState::Done;
    }

    bool isError() {
        return state == TaskEvalState::Error;
    }

    bool isDoneOrError() {
        return isDone() || isError();
    }

    static TaskEval idle() {
        return TaskEval{ TaskEvalState::Idle };
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
    virtual bool collection() {
        return false;
    }
    virtual void enqueued() { }
    virtual bool tryBegin() {
        return true;
    }
    virtual TaskEval task() = 0;
    virtual void done() { }
    virtual void error() { }

public:
    const char *toString() const {
        return name;
    }

    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void info(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void trace(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void warn(const char *f, ...) const __attribute__((format(printf, 2, 3)));

};

TaskEval callTask(Task *task);

inline bool areSame(const Task &a, const Task &b) {
    return &a == &b;
}

template<std::size_t Size>
class TaskCollection : public Task {
private:
    bool sequential { false };
    std::array<Task*, Size> tasks;

public:
    TaskCollection(bool sequential, std::array<Task*, Size>&& array) : Task("Tasks"), sequential(sequential), tasks(array) {
    }

public:
    bool collection() override {
        return true;
    }

    void enqueued() override {
        for (std::size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                tasks[i]->enqueued();
            }
        }
    }

    TaskEval task() override {
        auto e = TaskEval::done();

        for (std::size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                auto taskStatus = callTask(tasks[i]);
                if (!taskStatus.isDone()) {
                    e = TaskEval::busy();
                    if (sequential) {
                        break;
                    }
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

class TaskQueue : public Task {
public:
    TaskQueue(const char *name) : Task(name) {
    }

public:
    virtual bool prepend(Task &task) = 0;
    virtual bool append(Task &task) = 0;
};

}

#endif
