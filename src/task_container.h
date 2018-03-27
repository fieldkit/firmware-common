#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>
#include <utility>

namespace fk {

template<class T>
class TaskContainer : public Task {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
    T *target{ nullptr };

public:
    TaskContainer() : Task("TC") {
    }

    void enqueued() override {
        require()->enqueued();
    }

    void done() override {
        require()->done();
    }

    void error() override {
        require()->error();
    }

    TaskEval task() override {
        return require()->task();
    }

    template<typename ...Args>
    T &ready(Args &&...args) {
        target = new (&data) T(std::forward<Args>(args)...);
        return *target;
    }

private:
    Task *require() {
        fk_assert(target != nullptr);
        return target;
    }

};

}

#endif
