#ifndef TIMER_SERVICE_TEST_H_
#define TIMER_SERVICE_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class TimerServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void TimerServiceTest::SetUp() {}

void TimerServiceTest::TearDown() {}

void TimerServiceTest::SetUpTestCase() {}

void TimerServiceTest::TearDownTestCase() {}

#endif