#ifndef __AGENT_VMWARENATIVE_BACKUP_PREPARATION_TASK__
#define __AGENT_VMWARENATIVE_BACKUP_PREPARATION_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeBackupPreparationTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeBackupPreparationTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeBackupPreparationTaskTest::SetUp()
{}

void VMwareNativeBackupPreparationTaskTest::TearDown()
{}

void VMwareNativeBackupPreparationTaskTest::SetUpTestCase()
{}

void VMwareNativeBackupPreparationTaskTest::TearDownTestCase()
{}

#endif
