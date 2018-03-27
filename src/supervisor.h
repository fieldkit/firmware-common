#ifndef FK_SUPERVISOR_H_INCLUDED
#define FK_SUPERVISOR_H_INCLUDED

namespace fk {

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
                case TaskEvalState::Idle: {
                    break;
                }
                case TaskEvalState::Busy: {
                    break;
                }
                case TaskEvalState::Done: {
                    tasks[i]->done();
                    log("Done: %s", tasks[i]->name);
                    tasks[i] = nullptr;
                    break;
                }
                case TaskEvalState::Error: {
                    tasks[i]->error();
                    log("Error: %s", tasks[i]->name);
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

        if (!contains(task)) {
            for (size_t i = 0; i < Size; ++i) {
                if (tasks[i] == nullptr) {
                    log("Queuing: %s", task.name);
                    task.enqueued();
                    tasks[i] = &task;
                    return true;
                }
            }
        }

        log("Dropped: %s", task.name);
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr) {
                log("Queue[%d]: %s", i, tasks[i]->name);
            }
        }

        return false;
    }

private:
    void log(const char *f, ...) const __attribute__((format(printf, 2, 3))) {
        va_list args;
        va_start(args, f);
        vdebugfpln("Supervisor", f, args);
        va_end(args);
    }

    void shrink() {
        for (size_t i = 0; i < Size; ++i) {

        }
    }

    bool contains(Task &task) {
        for (size_t i = 0; i < Size; ++i) {
            if (tasks[i] != nullptr && tasks[i] == &task) {
                return true;
            }
        }
        return false;
    }

};

}

#endif
