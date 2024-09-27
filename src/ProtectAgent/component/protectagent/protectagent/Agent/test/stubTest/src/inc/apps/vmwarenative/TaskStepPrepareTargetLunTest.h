#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_TARGETLUN_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_TARGETLUN_H__

#include "apps/vmwarenative/TaskStepPrepareTargetLun.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepPrepareTargetLunTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepPrepareTargetLunTest::SetUp()
{}

void TaskStepPrepareTargetLunTest::TearDown()
{}

void TaskStepPrepareTargetLunTest::SetUpTestCase()
{}

void TaskStepPrepareTargetLunTest::TearDownTestCase()
{}

#endif
