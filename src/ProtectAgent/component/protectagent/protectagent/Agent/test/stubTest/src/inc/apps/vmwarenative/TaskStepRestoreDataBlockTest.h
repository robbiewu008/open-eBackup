#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_RESTORE_DATABLOCK_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_RESTORE_DATABLOCK_H__

#include "apps/vmwarenative/TaskStepRestoreDataBlock.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepRestoreDataBlockTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepRestoreDataBlockTest::SetUp()
{}

void TaskStepRestoreDataBlockTest::TearDown()
{}

void TaskStepRestoreDataBlockTest::SetUpTestCase()
{}

void TaskStepRestoreDataBlockTest::TearDownTestCase()
{}

#endif
