#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>
#include <utility>

namespace fk {

template<class T>
class TaskContainer {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
    T *target{ nullptr };

public:
    template<typename ...Args>
    T *ready(Args &&...args) {
        target = new (&data) T(std::forward<Args>(args)...);
        return target;
    }

private:
    void clear() {
        target = nullptr;
    }

};

class ActiveObject : public Task {
private:
    Task *tasks{ nullptr };

public:
    ActiveObject(const char *name);

public:
    TaskEval task() override;

public:
    bool inQueue(Task &task);
    void push(Task &task);
    void tick();
    bool isIdle();
    void cancel();

public:
    virtual void done(Task &task);
    virtual void error(Task &task);
    virtual void idle();

protected:
    void service(Task &task);

private:
    void pop();
    Task *tail();
    Task **end();

};

class ChildContainer {
private:
    Task *child { nullptr };

public:
    void push(Task &task) {
        child = &task;
        child->enqueued();
    }

    bool hasChild() {
        return child != nullptr;
    }

    TaskEval task();

    void clear() {
        child = nullptr;
    }


};

}

#endif
