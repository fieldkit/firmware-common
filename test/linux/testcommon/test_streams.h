#include <gtest/gtest.h>

#include "streams.h"

class StreamsSuite : public ::testing::Test {
protected:
    StreamsSuite();
    virtual ~StreamsSuite();

    virtual void SetUp();
    virtual void TearDown();

};
