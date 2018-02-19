#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>
#include <utility>

namespace fk {

template<class T>
class TaskContainer : public ActiveObject {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
    T *target{ nullptr };

public:
    TaskContainer() : ActiveObject("TC") {
    }

    void enqueued() override {
        if (target == nullptr) {
            log("Enqueued before being initialized");
        } else {
            push(*target);
        }
    }

    template<typename ...Args>
    TaskContainer &ready(Args &&...args) {
        target = new (&data) T(std::forward<Args>(args)...);
        return *this;
    }
};

}

#endif
