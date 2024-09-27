#ifndef __HOSTTEST_H__
#define __HOSTTEST_H__

#define private public

#include "common/Log.h"
#include "common/ErrorCode.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "gtest/gtest.h"

#define private public


class CHostTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};
void CHostTest::SetUp() {}
void CHostTest::TearDown() {}
void CHostTest::SetUpTestCase() {}
void CHostTest::TearDownTestCase() {}

#endif
