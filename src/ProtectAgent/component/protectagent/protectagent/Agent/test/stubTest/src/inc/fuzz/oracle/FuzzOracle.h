#ifndef _FUZZ_ORACLE_H_
#define _FUZZ_ORACLE_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzOracle : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzOracle::SetUp() {}

void FuzzOracle::TearDown() {}

void FuzzOracle::SetUpTestCase() {}

void FuzzOracle::TearDownTestCase() {}

#endif /* _AGENT_ORACLENATIVE_PLUGIN_TEST_H_; */

