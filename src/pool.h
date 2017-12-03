#ifndef FK_POOL_H_INCLUDED
#define FK_POOL_H_INCLUDED

#include <cstdint>
#include <cstdlib>

namespace fk {

class Pool final {
private:
    const char *name;
    void *block;
    void *ptr;
    size_t remaining;
    size_t size;

public:
    Pool(const char *name, size_t size, Pool *parent = nullptr);
    void clear();
    void *malloc(size_t size);
    void *copy(void *ptr, size_t size);
    virtual ~Pool();

};


#define __POOL_LINE_STR(x) #x
#define __POOL_LINE(x) __POOL_LINE_STR(x)
#define PoolHere(var, size) var(__FILE__ ":" __POOL_LINE(__LINE__), size)
}

#endif
