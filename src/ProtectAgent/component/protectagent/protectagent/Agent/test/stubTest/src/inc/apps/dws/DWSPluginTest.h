#ifndef _AGENT_DWS_PLUGIN_TEST_H_
#define _AGENT_DWS_PLUGIN_TEST_H_

#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "plugins/dws/DWSPlugin.h"

class DWSPluginTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    Stub stubDwsPlugin;
    std::shared_ptr<DWSPlugin> ptrDwsPlugin;
};

void DWSPluginTest::TearDown() {}

void DWSPluginTest::SetUpTestCase() {}

void DWSPluginTest::TearDownTestCase() {}

#endif /* _AGENT_DWS_PLUGIN_TEST_H_ */
