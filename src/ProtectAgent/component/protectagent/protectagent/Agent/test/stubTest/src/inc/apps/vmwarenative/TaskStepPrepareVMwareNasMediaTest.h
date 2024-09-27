#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_VMWARENASMEDIA_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_VMWARENASMEDIA_H__

#include "apps/vmwarenative/TaskStepPrepareVMwareNasMedia.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepPrepareVMwareNasMediaTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepPrepareVMwareNasMediaTest::SetUp()
{}

void TaskStepPrepareVMwareNasMediaTest::TearDown()
{}

void TaskStepPrepareVMwareNasMediaTest::SetUpTestCase()
{}

void TaskStepPrepareVMwareNasMediaTest::TearDownTestCase()
{}
#endif
