#ifndef _AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_TEST_H_
#define _AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_TEST_H_
#define private public
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeDismountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepOracleNativeDismountTest::SetUp()
{}

void TaskStepOracleNativeDismountTest::TearDown()
{}

void TaskStepOracleNativeDismountTest::SetUpTestCase()
{}

void TaskStepOracleNativeDismountTest::TearDownTestCase()
{}

#endif
