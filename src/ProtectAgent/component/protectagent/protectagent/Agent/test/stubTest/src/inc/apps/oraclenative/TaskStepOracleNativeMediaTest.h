#ifndef _AGENT_BACKUP_STEP_PREPARE_MEDIA_TEST_H_
#define _AGENT_BACKUP_STEP_PREPARE_MEDIA_TEST_H_
#define private public
#define protected public
#include "apps/oraclenative/TaskStepOracleNativeMedia.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeMediaTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    Stub stub;
};

void TaskStepOracleNativeMediaTest::SetUp()
{}

void TaskStepOracleNativeMediaTest::TearDown()
{}

void TaskStepOracleNativeMediaTest::SetUpTestCase()
{}

void TaskStepOracleNativeMediaTest::TearDownTestCase()
{}

#endif
