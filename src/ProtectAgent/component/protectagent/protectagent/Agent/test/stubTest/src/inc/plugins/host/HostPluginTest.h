#ifndef _AGENT_HOST_PLUGIN_TEST_H_
#define _AGENT_HOST_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "cunitpub/publicInc.h"

class CHostPluginTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void CHostPluginTest::SetUp() {}

void CHostPluginTest::TearDown() {}

void CHostPluginTest::SetUpTestCase() {}

void CHostPluginTest::TearDownTestCase() {}

#endif /* _AGENT_HOST_PLUGIN_TEST_H_; */
