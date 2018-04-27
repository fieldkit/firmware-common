#include <gtest/gtest.h>

#define LWS_ENABLE_PROTOBUF
#include <lwstreams/lwstreams.h>

class StreamsSuite : public ::testing::Test {
protected:
    StreamsSuite();
    virtual ~StreamsSuite();

    virtual void SetUp();
    virtual void TearDown();

};
