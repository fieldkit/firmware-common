#include "active_object.h"

namespace fk {

ActiveObject::ActiveObject() : name("AO") {
}

ActiveObject::ActiveObject(const char *name, Task &idleTask) : name(name), idleTask(&idleTask) {
}

void ActiveObject::push(Task &task) {
    log("%s enqueue", task.toString());
    *end() = &task;
    task.enqueued();
}

void ActiveObject::pop() {
    if (tasks != nullptr) {
        tasks = tasks->nextTask;
    }
}

void ActiveObject::service(Task &active) {
    auto e = active.task();
    if (e.isDone()) {
        log("%s done", active.toString());
        pop();
        active.nextTask = nullptr;
        active.done();
        done(active);
        if (e.task != nullptr) {
            push(*e.task);
        }
    } else if (e.isError()) {
        log("%s error", active.toString());
        pop();
        active.nextTask = nullptr;
        active.error();
        error(active);
        if (e.task != nullptr) {
            push(*e.task);
        }
    }
}

void ActiveObject::tick() {
    if (!idle()) {
        service(*tasks);
    } else if (idleTask != nullptr) {
        service(*idleTask);
    }
}

bool ActiveObject::idle() {
    return tasks == nullptr;
}

void ActiveObject::done(Task &task) {
}

void ActiveObject::error(Task &task) {
}

void ActiveObject::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln(name, f, args);
    va_end(args);
}

Task **ActiveObject::end() {
    if (tasks == nullptr) {
        return &tasks;
    }
    for (auto i = tasks;; i = i->nextTask) {
        if (i->nextTask == nullptr) {
            return &i->nextTask;
        }
    }
}

}
