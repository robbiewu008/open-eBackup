#ifndef _AGENT_BACKUP_STEP_CHECK_ORACLE_EXISTS_TEST_H_
#define _AGENT_BACKUP_STEP_CHECK_ORACLE_EXISTS_TEST_H_
#define private public
#include "apps/oraclenative/TaskStepCheckDBExists.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepCheckDBExistsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepCheckDBExistsTest::SetUp()
{}

void TaskStepCheckDBExistsTest::TearDown()
{}

void TaskStepCheckDBExistsTest::SetUpTestCase()
{}

void TaskStepCheckDBExistsTest::TearDownTestCase()
{}

#endif
