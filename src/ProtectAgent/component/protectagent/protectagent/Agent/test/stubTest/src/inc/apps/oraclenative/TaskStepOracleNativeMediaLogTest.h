#ifndef _AGENT_BACKUP_STEP_PREPARE_MEDIA_LOG_TEST_H_
#define _AGENT_BACKUP_STEP_PREPARE_MEDIA_LOG_TEST_H_
#define private public
#define protected public
#include "apps/oraclenative/TaskStepOracleNativeMediaLog.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeMediaLogTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepOracleNativeMediaLogTest::SetUp()
{}

void TaskStepOracleNativeMediaLogTest::TearDown()
{}

void TaskStepOracleNativeMediaLogTest::SetUpTestCase()
{}

void TaskStepOracleNativeMediaLogTest::TearDownTestCase()
{}

#endif
