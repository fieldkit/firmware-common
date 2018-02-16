#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>

namespace fk {

template<class T>
class TaskContainer : public ActiveObject {
private:
    using TaskContext = typename T::Context;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data[1];
    T *target;

public:
    TaskContainer() : ActiveObject("TC") {
    }

    void enqueued() override {
        push(*target);
    }

    TaskContainer &ready(TaskContext c) {
        target = new (data) T(c);
        return *this;
    }
};

}

#endif
