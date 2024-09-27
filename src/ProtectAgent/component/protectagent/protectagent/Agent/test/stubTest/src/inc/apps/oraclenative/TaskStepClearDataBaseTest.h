#ifndef _AGENT_BACKUP_STEP_CLEAR_DATABASE_TEST_H_
#define _AGENT_BACKUP_STEP_CLEAR_DATABASE_TEST_H_
#define private public
#include "apps/oraclenative/TaskStepClearDataBase.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepClearDataBaseTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepClearDataBaseTest::SetUp()
{}

void TaskStepClearDataBaseTest::TearDown()
{}

void TaskStepClearDataBaseTest::SetUpTestCase()
{}

void TaskStepClearDataBaseTest::TearDownTestCase()
{}

#endif
