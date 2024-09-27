#ifndef __AGENT_TASKSTEP_RECOVERY_PREPARATION_H__
#define __AGENT_TASKSTEP_RECOVERY_PREPARATION_H__

#include "apps/vmwarenative/TaskStepRecoveryPreparation.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepRecoveryPreparationTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepRecoveryPreparationTest::SetUp()
{}

void TaskStepRecoveryPreparationTest::TearDown()
{}

void TaskStepRecoveryPreparationTest::SetUpTestCase()
{}

void TaskStepRecoveryPreparationTest::TearDownTestCase()
{}

#endif
