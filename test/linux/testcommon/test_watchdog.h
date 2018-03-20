#include <gtest/gtest.h>

class WatchdogSuite : public ::testing::Test {
protected:
    WatchdogSuite();
    virtual ~WatchdogSuite();

    virtual void SetUp();
    virtual void TearDown();

};
