#ifndef __AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK__
#define __AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeRestorePreparationTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeRestorePreparationTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeRestorePreparationTaskTest::SetUp()
{}

void VMwareNativeRestorePreparationTaskTest::TearDown()
{}

void VMwareNativeRestorePreparationTaskTest::SetUpTestCase()
{}

void VMwareNativeRestorePreparationTaskTest::TearDownTestCase()
{}

#endif
