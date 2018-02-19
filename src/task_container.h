#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>
#include <utility>

namespace fk {

template<class T>
class TaskContainer : public ActiveObject {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data[1];
    T *target;

public:
    TaskContainer() : ActiveObject("TC") {
    }

    void enqueued() override {
        push(*target);
    }

    template<typename ...Args>
    TaskContainer &ready(Args &&...args) {
        target = new (data) T(std::forward<Args>(args)...);
        return *this;
    }
};

}

#endif
