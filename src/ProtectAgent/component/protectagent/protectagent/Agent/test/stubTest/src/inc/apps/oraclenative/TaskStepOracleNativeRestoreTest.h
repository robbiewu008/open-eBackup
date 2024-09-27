#ifndef _AGENT_BACKUP_STEP_RESTORE_ORACLE_TEST_H_
#define _AGENT_BACKUP_STEP_RESTORE_ORACLE_TEST_H_
#define private public
#define protected public
#include "apps/oraclenative/TaskStepOracleNativeRestore.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeRestoreTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
 
    Stub stub;
};

void TaskStepOracleNativeRestoreTest::SetUp()
{}

void TaskStepOracleNativeRestoreTest::TearDown()
{}

void TaskStepOracleNativeRestoreTest::SetUpTestCase()
{}

void TaskStepOracleNativeRestoreTest::TearDownTestCase()
{}

#endif
