#ifndef __AGENT_VMWARENATIVE_VMBACKUP_TASK__
#define __AGENT_VMWARENATIVE_VMBACKUP_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeVmBackupTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeVmBackupTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeVmBackupTaskTest::SetUp()
{}

void VMwareNativeVmBackupTaskTest::TearDown()
{}

void VMwareNativeVmBackupTaskTest::SetUpTestCase()
{}

void VMwareNativeVmBackupTaskTest::TearDownTestCase()
{}

#endif
