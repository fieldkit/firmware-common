#include "active_object.h"

namespace fk {

TaskEval TaskEval::Error;
TaskEval TaskEval::Idle;
TaskEval TaskEval::Done;
TaskEval TaskEval::Pass;

ActiveObject::ActiveObject() {
}

ActiveObject::ActiveObject(Task &idleTask) : idleTask(&idleTask ) {
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
    auto& e = active.task();
    if (areSame(e, TaskEval::Done)) {
        log("%s done", active.toString());
        pop();
        active.nextTask = nullptr;
        active.done();
        done(active);
    }
    else if (areSame(e, TaskEval::Error)) {
        log("%s error", active.toString());
        pop();
        active.nextTask = nullptr;
        active.error();
        error(active);
    }
    else if (e.task != nullptr) {
        log("%s done (passing)", active.toString());
        pop();
        active.nextTask = nullptr;
        active.done();
        done(active);
        push(*e.task);
    }
}

void ActiveObject::tick() {
    if (!idle()) {
        service(*tasks);
    }
    else if (idleTask != nullptr) {
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
    vdebugfpln("AO", f, args);
    va_end(args);
}

Task **ActiveObject::end() {
    if (tasks == nullptr) {
        return &tasks;
    }
    for (auto i = tasks; ; i = i->nextTask) {
        if (i->nextTask == nullptr) {
            return &i->nextTask;
        }
    }
}

}
