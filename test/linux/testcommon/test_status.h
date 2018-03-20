#include <gtest/gtest.h>

#include "status.h"

class StatusSuite : public ::testing::Test {
protected:
    StatusSuite();
    virtual ~StatusSuite();

    virtual void SetUp();
    virtual void TearDown();

};
