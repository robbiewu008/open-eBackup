#ifndef _AGENT_BACKUP_STEP_PREPARE_MEDIA_DATA_TEST_H_
#define _AGENT_BACKUP_STEP_PREPARE_MEDIA_DATA_TEST_H_

#define protected public
#define private public
#include "apps/oraclenative/TaskStepOracleNativeMediaData.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeMediaDataTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    Stub stub;
};

void TaskStepOracleNativeMediaDataTest::SetUp()
{}

void TaskStepOracleNativeMediaDataTest::TearDown()
{}

void TaskStepOracleNativeMediaDataTest::SetUpTestCase()
{}

void TaskStepOracleNativeMediaDataTest::TearDownTestCase()
{}

#endif
