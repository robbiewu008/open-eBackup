#ifndef _AGENT_HOSTAGENT_PLUGIN_TEST_H_
#define _AGENT_HOSTAGENT_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"

class FuzzHostAgent : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub m_stub;
};

void FuzzHostAgent::SetUp() {}

void FuzzHostAgent::TearDown() {}

void FuzzHostAgent::SetUpTestCase() {}

void FuzzHostAgent::TearDownTestCase() {}
#endif 