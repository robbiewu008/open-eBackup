#ifndef __DATA_CONTEXT_TEST_H__
#define __DATA_CONTEXT_TEST_H__

#include "dataprocess/datareadwrite/DataContext.h"
#include "gtest/gtest.h"
#include "stub.h"

class DataContextTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void DataContextTest::SetUp() {}

void DataContextTest::TearDown() {}

void DataContextTest::SetUpTestCase() {}

void DataContextTest::TearDownTestCase() {}

#endif