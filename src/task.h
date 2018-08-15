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

inline bool simple_task_run(Task &task)  {
    auto e = task.task();
    if (e.isDone()) {
        task.done();
        return false;
    }
    if (e.isError()) {
        task.error();
        return false;
    }
    return true;
}

}

#endif
