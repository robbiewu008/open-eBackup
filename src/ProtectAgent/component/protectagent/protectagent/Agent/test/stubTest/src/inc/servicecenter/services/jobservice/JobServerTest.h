#ifndef JOB_SERVICE_TEST_H_
#define JOB_SERVICE_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class JobServerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void JobServerTest::SetUp() {}

void JobServerTest::TearDown() {}

void JobServerTest::SetUpTestCase() {}

void JobServerTest::TearDownTestCase() {}

#endif