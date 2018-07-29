#ifndef FK_POOL_H_INCLUDED
#define FK_POOL_H_INCLUDED

#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace fk {

constexpr size_t AlignedOn = 4;

constexpr size_t alignedSize(size_t size) {
    return size + (AlignedOn - (size % AlignedOn));
}

class Pool {
private:
    const char *name;
    void *block;
    void *ptr;
    size_t remaining;
    size_t size;

public:
    Pool(const char *name, size_t size, void *block);

public:
    void clear();
    void *malloc(size_t size);
    void *copy(void *ptr, size_t size);

};

template<size_t N>
class StaticPool : public Pool {
private:
    typename std::aligned_storage<sizeof(uint8_t), alignof(uint8_t)>::type data[alignedSize(N)];

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
