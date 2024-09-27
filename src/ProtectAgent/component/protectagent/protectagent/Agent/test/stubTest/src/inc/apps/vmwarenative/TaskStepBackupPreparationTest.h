#ifndef __AGENT_TASKSTEP_BACKUP_PREPARATION_H__
#define __AGENT_TASKSTEP_BACKUP_PREPARATION_H__

#include "apps/vmwarenative/TaskStepBackupPreparation.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepBackupPreparationTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepBackupPreparationTest::SetUp()
{}

void TaskStepBackupPreparationTest::TearDown()
{}

void TaskStepBackupPreparationTest::SetUpTestCase()
{}

void TaskStepBackupPreparationTest::TearDownTestCase()
{}

#endif
