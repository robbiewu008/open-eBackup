#ifndef _AGENT_BACKUP_STEP_CHECK_ORACLE_CLOSE_TEST_H_
#define _AGENT_BACKUP_STEP_CHECK_ORACLE_CLOSE_TEST_H_
#define private public
#include "apps/oraclenative/TaskStepCheckDBClose.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepCheckDBCloseTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepCheckDBCloseTest::SetUp()
{}

void TaskStepCheckDBCloseTest::TearDown()
{}

void TaskStepCheckDBCloseTest::SetUpTestCase()
{}

void TaskStepCheckDBCloseTest::TearDownTestCase()
{}

#endif
