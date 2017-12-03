#include "active_object.h"

namespace fk {

TaskEval TaskEval::Error;
TaskEval TaskEval::Idle;
TaskEval TaskEval::Done;
TaskEval TaskEval::Pass;

ActiveObject::ActiveObject() {
}

void ActiveObject::push(Task &task) {
    log("%s enqueue", task.toString());
    *end() = &task;
    task.enqueued();
}

void ActiveObject::tick() {
    if (!idle()) {
        auto& curr = *tasks;
        auto& e = curr.task();
        if (areSame(e, TaskEval::Done)) {
            log("%s done", curr.toString());
            tasks = tasks->nextTask;
            curr.nextTask = nullptr;
            curr.done();
            done(curr);
        }
        else if (areSame(e, TaskEval::Error)) {
            log("%s error", curr.toString());
            tasks = tasks->nextTask;
            curr.nextTask = nullptr;
            curr.error();
            error(curr);
        }
        else if (e.task != nullptr) {
            log("%s done (pass)", curr.toString());
            tasks = tasks->nextTask;
            curr.nextTask = nullptr;
            curr.done();
            done(curr);
            push(*e.task);
        }
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
