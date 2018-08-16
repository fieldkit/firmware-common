#ifndef FK_POOL_H_INCLUDED
#define FK_POOL_H_INCLUDED

#include <cinttypes>
#include <cstdlib>
#include <type_traits>

namespace fk {

constexpr size_t AlignedOn = 4;

constexpr size_t alignedSize(const size_t size) {
    return (size % AlignedOn != 0) ? (size + (AlignedOn - (size % AlignedOn))) : size;
}

class Pool {
private:
    const char *name_;
    void *block_;
    void *ptr_;
    size_t remaining_;
    size_t size_;
    bool frozen_{ false };

public:
    Pool(const char *name, size_t size, void *block);

public:
    size_t allocated() const {
        return size_ - remaining_;
    }

    size_t size() const {
        return size_;
    }

    bool frozen() const {
        return frozen_;
    }

    void clear();
    void *malloc(size_t size);
    void *copy(void *ptr, size_t size);
    char *strdup(const char *str);
    Pool freeze(const char *name);

};

template<size_t N>
class StaticPool : public Pool {
private:
    alignas(sizeof(uint32_t)) typename std::aligned_storage<sizeof(uint8_t), alignof(uint8_t)>::type data[alignedSize(N)];

public:
    StaticPool(const char *name) : Pool(name, alignedSize(N), (void *)data) {
    }

};

class EmptyPool : public Pool {
public:
    EmptyPool() : Pool("Empty", 0, nullptr) {
    }

};

#define __POOL_LINE_STR(x) #x
#define __POOL_LINE(x) __POOL_LINE_STR(x)
#define PoolHere(var, size) var(__FILE__ ":" __POOL_LINE(__LINE__), size)

}

#endif
