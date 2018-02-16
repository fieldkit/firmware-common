#include "active_object.h"

namespace fk {

void Task::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln(name, f, args);
    va_end(args);
}

ActiveObject::ActiveObject(const char *name) : Task(name) {
}

ActiveObject::ActiveObject(const char *name, Task &idleTask) : Task(name), idleTask(&idleTask) {
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
        log("%s enqueue skipped (already queued)", task.toString());
        return;
    }

    log("%s enqueue (%s)", task.toString(), tasks == nullptr ? "HEAD" : tail()->toString());
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
    return TaskEval::idle();
}

void ActiveObject::service(Task &active) {
    auto e = active.task();

    if (e.isYield()) {
        pop();
        active.nextTask = nullptr;
        idle();
        if (e.nextTask != nullptr) {
            *end() = e.nextTask;
        }
        *end() = &active;
    } else if (e.isDone()) {
        log("%s done", active.toString());
        pop();
        active.nextTask = nullptr;
        active.done();
        done(active);
        if (e.nextTask != nullptr) {
            push(*e.nextTask);
        }
    } else if (e.isError()) {
        log("%s error", active.toString());
        pop();
        active.nextTask = nullptr;
        active.error();
        error(active);
        if (e.nextTask != nullptr) {
            push(*e.nextTask);
        }
    }
}

void ActiveObject::tick() {
    auto began = millis();
    auto ran = "Idle";
    if (!isIdle()) {
        ran = tasks->toString();
        service(*tasks);
    } else if (idleTask != nullptr) {
        ran = idleTask->toString();
        service(*idleTask);
        idle();
    } else {
        idle();
    }
    auto ended = millis();
    if (ended - began > 500) {
        log("Long tick from %s (%lu) (****)", ran, ended - began);
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

}
