#ifndef FK_SUPERVISOR_H_INCLUDED
#define FK_SUPERVISOR_H_INCLUDED

namespace fk {

template<std::size_t Size>
class Supervisor : public TaskQueue {
private:
    size_t size{ Size };
    bool sequential{ false };
    std::array<Task*, Size> tasks;

public:
    Supervisor(bool sequential = false) : TaskQueue("Supervisor"), sequential(sequential) {
        for (size_t i = 0; i < Size; ++i) {
            tasks[i] = nullptr;
        }
    }

public:
    bool collection() override {
        return true;
    }

    TaskEval task() override {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                auto status = callTask(tasks[i]);
                switch (status.state) {
                case TaskEvalState::Idle: {
                    break;
                }
                case TaskEvalState::Busy: {
                    break;
                }
                case TaskEvalState::Done: {
                    tasks[i]->done();
                    #ifdef FK_SUPERVISOR_LOGGING_VERBOSE
                    log("Done: %s", tasks[i]->name);
                    #endif
                    tasks[i] = nullptr;
                    shrink(i);
                    break;
                }
                case TaskEvalState::Error: {
                    tasks[i]->error();
                    log("Error: %s", tasks[i]->name);
                    tasks[i] = nullptr;
                    shrink(i);
                    break;
                }
                }

                if (sequential) {
                    break;
                }
            }
        }
        return TaskEval::busy();
    }

    bool prepend(Task &task) override {
        if (fullOrAlreadyHas(task)) {
            log("Dropped: %s", task.name);
            logQueue();
            return false;
        }

        #ifdef FK_SUPERVISOR_LOGGING_VERBOSE
        log("Queuing: %s", task.name);
        #endif
        for (size_t i = Size - 1; i > 0; --i) {
            tasks[i] = tasks[i - 1];
        }
        task.enqueued();
        tasks[0] = &task;
        return true;
    }

    bool append(Task &task) override {
        if (fullOrAlreadyHas(task)) {
            log("Dropped: %s", task.name);
            logQueue();
            return false;
        }

        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] == nullptr) {
                #ifdef FK_SUPERVISOR_LOGGING_VERBOSE
                log("Queuing: %s", task.name);
                #endif
                task.enqueued();
                tasks[i] = &task;
                return true;
            }
        }

        return false;
    }

private:
    void logQueue() {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                log("Queue[%d]: %s", i, tasks[i]->name);
            }
        }
    }

    bool full() {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] == nullptr) {
                return false;
            }
        }
        return true;
    }

    bool fullOrAlreadyHas(Task &task) {
        return contains(task) || full();
    }

    void shrink(size_t index) {
        for (size_t i = index; i < Size - 1; ++i) {
            tasks[i] = tasks[i + 1];
        }
        tasks[Size - 1] = nullptr;
    }

    bool contains(Task &task) {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr && tasks[i] == &task) {
                return true;
            }
        }
        return false;
    }

    void log(const char *f, ...) const __attribute__((format(printf, 2, 3))) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::INFO, "Supervisor", f, args);
        va_end(args);
    }

};

}

#endif
