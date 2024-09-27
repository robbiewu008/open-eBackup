#ifndef __AGENT_CPU_TEST_H__
#define __AGENT_CPU_TEST_H__

#define private public
#define protected public

#include "host/Cpu.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "securecom/CryptAlg.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace std;

class  CpuTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};
void CpuTest::SetUp() {}
void CpuTest::TearDown() {}
void CpuTest::SetUpTestCase() {}
void CpuTest::TearDownTestCase() {}
#endif