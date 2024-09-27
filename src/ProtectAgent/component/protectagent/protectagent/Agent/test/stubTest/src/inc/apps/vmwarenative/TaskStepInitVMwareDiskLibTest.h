#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H__

#include "apps/vmwarenative/TaskStepInitVMwareDiskLib.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepInitVMwareDiskLibTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepInitVMwareDiskLibTest::SetUp()
{}

void TaskStepInitVMwareDiskLibTest::TearDown()
{}

void TaskStepInitVMwareDiskLibTest::SetUpTestCase()
{}

void TaskStepInitVMwareDiskLibTest::TearDownTestCase()
{}

#endif
