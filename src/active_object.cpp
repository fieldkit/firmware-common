#include "active_object.h"

namespace fk {

void callIdle(ActiveObject *task) {
    auto began = fk_uptime();
    task->idle();
    auto ended = fk_uptime();
    if (ended - began > 500) {
        logf(LogLevels::TRACE, "Tasks", "Long idle: %s (%lu)", task->name, ended - began);
    }
}

TaskEval callTask(Task *task) {
    auto began = fk_uptime();
    auto eval = task->task();
    auto ended = fk_uptime();
    if (!task->collection()) {
        if (ended - began > 500) {
            logf(LogLevels::TRACE, "Tasks", "Long task: %s (%lu)", task->name, ended - began);
        }
    }
    return eval;
}

void Task::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name, f, args);
    va_end(args);
}

void Task::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name, f, args);
    va_end(args);
}

void Task::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::TRACE, name, f, args);
    va_end(args);
}

void Task::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::WARN, name, f, args);
    va_end(args);
}

ActiveObject::ActiveObject(const char *name) : Task(name) {
}

bool ActiveObject::inQueue(Task &task) {
    // This should also return true if the Task is in another chain. Probably
    // should be a flag on the Task itself.
    for (auto iter = tasks; iter != nullptr; iter = iter->nextTask) {
        if (iter == &task) {
            return true;
        }
    }

    return false;
}

void ActiveObject::push(Task &task) {
    if (inQueue(task)) {
        info("%s enqueue skipped (already queued)", task.toString());
        return;
    }

    info("%s enqueue (%s)", task.toString(), tasks == nullptr ? "HEAD" : tail()->toString());
    *end() = &task;
    task.enqueued();
}

void ActiveObject::pop() {
    if (tasks != nullptr) {
        tasks = tasks->nextTask;
    }
}

TaskEval ActiveObject::task() {
    tick();
    if (isIdle()) {
        return TaskEval::done();
    }
    return TaskEval::busy();
}

void ActiveObject::service(Task &active) {
    auto e = callTask(&active);

    if (e.isDone()) {
        info("%s done", active.toString());
        pop();
        active.nextTask = nullptr;
        active.done();
        done(active);
    } else if (e.isError()) {
        warn("%s error", active.toString());
        pop();
        active.nextTask = nullptr;
        active.error();
        error(active);
    }
}

void ActiveObject::tick() {
    if (!isIdle()) {
        service(*tasks);
    } else {
        callIdle(this);
    }
}

bool ActiveObject::isIdle() {
    return tasks == nullptr;
}

void ActiveObject::done(Task &) {
}

void ActiveObject::error(Task &) {
}

void ActiveObject::idle() {
}

void ActiveObject::cancel() {
    while (tail() != nullptr) {
        pop();
    }
}

Task *ActiveObject::tail() {
    if (tasks == nullptr) {
        return nullptr;
    }
    for (auto i = tasks;; i = i->nextTask) {
        if (i->nextTask == nullptr) {
            return i;
        }
    }
}

Task **ActiveObject::end() {
    if (tasks == nullptr) {
        return &tasks;
    }
    return &(tail()->nextTask);
}

TaskEval ChildContainer::task() {
    if (child == nullptr) {
        return TaskEval::idle();
    }

    auto e = callTask(child);
    if (e.isDone()) {
        child->done();
        clear();
        return e;
    }
    else if (e.isError()) {
        child->error();
        clear();
        return e;
    }

    return e;
}

}
