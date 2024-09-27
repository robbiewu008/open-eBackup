#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_H__

#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepVMwareNativeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepVMwareNativeTest::SetUp()
{}

void TaskStepVMwareNativeTest::TearDown()
{}

void TaskStepVMwareNativeTest::SetUpTestCase()
{}

void TaskStepVMwareNativeTest::TearDownTestCase()
{}

#endif
