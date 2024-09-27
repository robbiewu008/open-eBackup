#ifndef _AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_TEST_H_
#define _AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_TEST_H_
#define private public
#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeExpireCopyTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    Stub stub;
};

void TaskStepOracleNativeExpireCopyTest::SetUp()
{}

void TaskStepOracleNativeExpireCopyTest::TearDown()
{}

void TaskStepOracleNativeExpireCopyTest::SetUpTestCase()
{}

void TaskStepOracleNativeExpireCopyTest::TearDownTestCase()
{}

#endif
