#ifndef _AGENT_BACKUP_STEP_LIVEMOUNT_TEST_ORACLE_
#define _AGENT_BACKUP_STEP_LIVEMOUNT_TEST_ORACLE_
#define private public
#include "apps/oraclenative/TaskStepOracleNativeLiveMount.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeLiveMountTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepOracleNativeLiveMountTest::SetUp()
{}

void TaskStepOracleNativeLiveMountTest::TearDown()
{}

void TaskStepOracleNativeLiveMountTest::SetUpTestCase()
{}

void TaskStepOracleNativeLiveMountTest::TearDownTestCase()
{}

#endif
