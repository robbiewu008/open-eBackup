#ifndef __AGENT_VMWARENATIVE_DEF_H__
#define __AGENT_VMWARENATIVE_DEF_H__

#include "apps/vmwarenative/VMwareDef.h"
#include "gtest/gtest.h"
#include "stub.h"

class OracleNativeDismountTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void OracleNativeDismountTaskTest::SetUp()
{}

void OracleNativeDismountTaskTest::TearDown()
{}

void OracleNativeDismountTaskTest::SetUpTestCase()
{}

void OracleNativeDismountTaskTest::TearDownTestCase()
{}
#endif
