#ifndef _AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#define _AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#include "apps/oraclenative/TaskStepOracleNativeCLiveMount.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeCLiveMountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepOracleNativeCLiveMountTest::SetUp()
{}

void TaskStepOracleNativeCLiveMountTest::TearDown()
{}

void TaskStepOracleNativeCLiveMountTest::SetUpTestCase()
{}

void TaskStepOracleNativeCLiveMountTest::TearDownTestCase()
{}

#endif
