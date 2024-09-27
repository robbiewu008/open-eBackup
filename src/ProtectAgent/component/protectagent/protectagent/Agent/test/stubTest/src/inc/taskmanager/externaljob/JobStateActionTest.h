#ifndef _JOB_STATE_ACTION_TEST_H_
#define _JOB_STATE_ACTION_TEST_H_

#define protected public
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class JobStateActionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void JobStateActionTest::SetUp() {}

void JobStateActionTest::TearDown() {}

void JobStateActionTest::SetUpTestCase() {}

void JobStateActionTest::TearDownTestCase() {}

#endif