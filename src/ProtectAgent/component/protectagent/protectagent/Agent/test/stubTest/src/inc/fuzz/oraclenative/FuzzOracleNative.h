#ifndef _FUZZ_ORACLE_NATIVE_H_
#define _FUZZ_ORACLE_NATIVE_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzOracleNative : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzOracleNative::SetUp() {}

void FuzzOracleNative::TearDown() {}

void FuzzOracleNative::SetUpTestCase() {}

void FuzzOracleNative::TearDownTestCase() {}

#endif /* _AGENT_ORACLENATIVE_PLUGIN_TEST_H_; */

