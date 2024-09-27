
#ifndef _CMPPIPETEST_H_
#define _CMPPIPETEST_H_
#define private public
#include "common/Uuid.h"
#include "gtest/gtest.h"
#include "common/Pipe.h"
#include "stub.h"

class CMpPipeTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void CMpPipeTest::SetUp() {}

void CMpPipeTest::TearDown() {}

void CMpPipeTest::SetUpTestCase() {}

void CMpPipeTest::TearDownTestCase() {}

#endif