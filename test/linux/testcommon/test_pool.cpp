#include <gtest/gtest.h>

#include "pool.h"

using namespace fk;

class PoolSuite : public ::testing::Test {
protected:

};

TEST_F(PoolSuite, Basic) {
    StaticPool<1024> pool("Pool");

    void *p1 = pool.malloc(256);
    ASSERT_EQ(pool.allocated(), 256);

    void *p2 = pool.malloc(256);
    ASSERT_EQ(pool.allocated(), 512);

    pool.clear();
    ASSERT_EQ(pool.allocated(), 0);
}

TEST_F(PoolSuite, Alignment) {
    StaticPool<1024> pool("Pool");

    void *p1 = pool.malloc(13);
    ASSERT_EQ(pool.allocated(), 16);
}

TEST_F(PoolSuite, Subpool) {
    StaticPool<2048> pool("Pool");

    void *p1 = pool.malloc(256);
    ASSERT_EQ(pool.allocated(), 256);
    ASSERT_FALSE(pool.frozen());

    Pool child = pool.freeze("Child");
    ASSERT_EQ(child.size(), 2048 - 256);
    ASSERT_EQ(child.allocated(), 0);

    ASSERT_TRUE(pool.frozen());

    pool.clear();
    ASSERT_FALSE(pool.frozen());
}
