#ifndef _AGENT_BACKUP_STEP_MOVE_DBF_TEST_ORACLE_
#define _AGENT_BACKUP_STEP_MOVE_DBF_TEST_ORACLE_
#define private public
#define protected public
#include "apps/oraclenative/TaskStepOracleNativeMoveDBF.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeMoveDBFTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
 
    Stub stub;
};

void TaskStepOracleNativeMoveDBFTest::SetUp()
{}

void TaskStepOracleNativeMoveDBFTest::TearDown()
{}

void TaskStepOracleNativeMoveDBFTest::SetUpTestCase()
{}

void TaskStepOracleNativeMoveDBFTest::TearDownTestCase()
{}

#endif