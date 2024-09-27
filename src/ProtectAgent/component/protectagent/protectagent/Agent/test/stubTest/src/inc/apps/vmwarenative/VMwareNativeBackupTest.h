#ifndef __AGENT_VMWARENATIVE_BACKUP__
#define __AGENT_VMWARENATIVE_BACKUP__

#include "apps/vmwarenative/VMwareNativeBackup.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeBackupTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeBackupTest::SetUp()
{}

void VMwareNativeBackupTest::TearDown()
{}

void VMwareNativeBackupTest::SetUpTestCase()
{}

void VMwareNativeBackupTest::TearDownTestCase()
{}

#endif
