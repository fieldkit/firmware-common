#ifndef FK_TASK_CONTAINER_H_INCLUDED
#define FK_TASK_CONTAINER_H_INCLUDED

#include <new>
#include <type_traits>
#include <utility>
#include <tuple>
#include "cpp-std-fill.h"

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

template<class T, size_t Size, typename ...Args>
class SimpleQueue : public Task {
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
    std::array<std::tuple<Args...>, Size> queue;
    T *target{ nullptr };
    size_t position{ 0 };

public:
    SimpleQueue(std::array<std::tuple<Args...>, Size> queue) : Task("SimpleQueue"), queue(queue) {
    }

    void enqueued() override {
        log("Enqueued");
        target = nullptr;
        position = 0;
    }

    TaskEval task() override {
        auto e = target == nullptr ? TaskEval::done() : target->task();
        if (e.isDone()) {
            if (target != nullptr) {
                target->~T();
                target = nullptr;
            }
            if (position == Size) {
                return TaskEval::done();
            }
            else {
                target = makeAt(&data);
                target->enqueued();
                position++;
            }
        }
        return TaskEval::busy();
    }

private:
    template<typename W>
    T *makeAt(W where) {
        return make_from_tuple_at<T>(where, queue[position]);
    }

};

template<class T, size_t Size, typename ...Args>
auto to_simple_queue(std::array<std::tuple<Args...>, Size> queue) -> SimpleQueue<T, Size, Args...> {
    return SimpleQueue<T, Size, Args...>(queue);
}

}

#endif
