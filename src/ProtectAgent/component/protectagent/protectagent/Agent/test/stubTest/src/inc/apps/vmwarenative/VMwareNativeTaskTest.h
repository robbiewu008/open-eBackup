#ifndef _AGENT_VMWARENATIVE_TASK_
#define _AGENT_VMWARENATIVE_TASK_

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeTaskTest::SetUp()
{}

void VMwareNativeTaskTest::TearDown()
{}

void VMwareNativeTaskTest::SetUpTestCase()
{}

void VMwareNativeTaskTest::TearDownTestCase()
{}

#endif
