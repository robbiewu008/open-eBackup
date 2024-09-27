#ifndef MESSAGESERVICETEST_H_
#define MESSAGESERVICETEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class MessageServiceTest :  public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void MessageServiceTest::SetUp() {}

void MessageServiceTest::TearDown() {}

void MessageServiceTest::SetUpTestCase() {}

void MessageServiceTest::TearDownTestCase() {}


#endif