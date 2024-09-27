#ifndef __AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK__
#define __AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeInitVddkLibTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeInitVddkLibTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeInitVddkLibTaskTest::SetUp()
{}

void VMwareNativeInitVddkLibTaskTest::TearDown()
{}

void VMwareNativeInitVddkLibTaskTest::SetUpTestCase()
{}

void VMwareNativeInitVddkLibTaskTest::TearDownTestCase()
{}

#endif
