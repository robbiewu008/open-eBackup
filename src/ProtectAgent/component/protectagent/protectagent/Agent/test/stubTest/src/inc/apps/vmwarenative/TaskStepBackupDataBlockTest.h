#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_BACKUP_DATABLOCK_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_BACKUP_DATABLOCK_H__

#include "apps/vmwarenative/TaskStepBackupDataBlock.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepBackupDataBlockTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepBackupDataBlockTest::SetUp()
{}

void TaskStepBackupDataBlockTest::TearDown()
{}

void TaskStepBackupDataBlockTest::SetUpTestCase()
{}

void TaskStepBackupDataBlockTest::TearDownTestCase()
{}

#endif
