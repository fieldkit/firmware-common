#include "active_object.h"

namespace fk {

void Task::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln(name, f, args);
    va_end(args);
}

ActiveObject::ActiveObject() : Task("AO") {
}

ActiveObject::ActiveObject(const char *name) : Task(name) {
}

ActiveObject::ActiveObject(const char *name, Task &idleTask) : Task(name), idleTask(&idleTask) {
}

void ActiveObject::push(Task &task) {
    log("%s enqueue", task.toString());
    *end() = &task;
    task.enqueued();
}

void ActiveObject::pop() {
    if (tasks != nullptr) {
        tasks = tasks->tasks;
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
        active.tasks = nullptr;
        idle();
        if (e.task != nullptr) {
            *end() = e.task;
        }
        *end() = &active;
    } else if (e.isDone()) {
        log("%s done", active.toString());
        pop();
        active.tasks = nullptr;
        active.done();
        done(active);
        if (e.task != nullptr) {
            push(*e.task);
        }
    } else if (e.isError()) {
        log("%s error", active.toString());
        pop();
        active.tasks = nullptr;
        active.error();
        error(active);
        if (e.task != nullptr) {
            push(*e.task);
        }
    }
}

void ActiveObject::tick() {
    if (!isIdle()) {
        service(*tasks);
    } else if (idleTask != nullptr) {
        service(*idleTask);
        idle();
    } else {
        idle();
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

Task **ActiveObject::end() {
    if (tasks == nullptr) {
        return &tasks;
    }
    for (auto i = tasks;; i = i->tasks) {
        if (i->tasks == nullptr) {
            return &i->tasks;
        }
    }
}

}
