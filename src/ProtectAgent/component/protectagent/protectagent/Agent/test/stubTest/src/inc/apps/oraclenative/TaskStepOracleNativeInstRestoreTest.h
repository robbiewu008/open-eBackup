#ifndef _AGENT_BACKUP_STEP_INST_RECOVER_TEST_ORACLE_
#define _AGENT_BACKUP_STEP_INST_RECOVER_TEST_ORACLE_
#define private public
#include "apps/oraclenative/TaskStepOracleNativeInstRestore.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeInstRestoreTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskStepOracleNativeInstRestoreTest::SetUp()
{}

void TaskStepOracleNativeInstRestoreTest::TearDown()
{}

void TaskStepOracleNativeInstRestoreTest::SetUpTestCase()
{}

void TaskStepOracleNativeInstRestoreTest::TearDownTestCase()
{}

#endif
