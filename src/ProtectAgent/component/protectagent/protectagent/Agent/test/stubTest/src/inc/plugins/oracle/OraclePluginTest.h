#ifndef _AGENT_ORACLE_PLUGIN_TEST_H_
#define _AGENT_ORACLE_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "cunitpub/publicInc.h"

class COraclePluginTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void COraclePluginTest::SetUp() {}

void COraclePluginTest::TearDown() {}

void COraclePluginTest::SetUpTestCase() {}

void COraclePluginTest::TearDownTestCase() {}


#endif /* _AGENT_ORACLE_PLUGIN_TEST_H_; */

