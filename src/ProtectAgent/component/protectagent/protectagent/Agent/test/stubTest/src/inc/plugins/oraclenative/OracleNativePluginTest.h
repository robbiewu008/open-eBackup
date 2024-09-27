#ifndef _AGENT_ORACLENATIVE_PLUGIN_TEST_H_
#define _AGENT_ORACLENATIVE_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "cunitpub/publicInc.h"

class COracleNativeBackupPluginTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void COracleNativeBackupPluginTest::SetUp() {}

void COracleNativeBackupPluginTest::TearDown() {}

void COracleNativeBackupPluginTest::SetUpTestCase() {}

void COracleNativeBackupPluginTest::TearDownTestCase() {}

#endif /* _AGENT_ORACLENATIVE_PLUGIN_TEST_H_; */

