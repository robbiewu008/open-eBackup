#ifndef __AGENT_VMWARENATIVE_VMRESTORE_TASK__
#define __AGENT_VMWARENATIVE_VMRESTORE_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeVmRestoreTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeVmRestoreTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeVmRestoreTaskTest::SetUp()
{}

void VMwareNativeVmRestoreTaskTest::TearDown()
{}

void VMwareNativeVmRestoreTaskTest::SetUpTestCase()
{}

void VMwareNativeVmRestoreTaskTest::TearDownTestCase()
{}

#endif
