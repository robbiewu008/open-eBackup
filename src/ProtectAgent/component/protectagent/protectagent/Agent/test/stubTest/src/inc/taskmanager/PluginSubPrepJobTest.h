#ifndef _PLUGIN_SUB_PRE_JOB_TEST_H_
#define _PLUGIN_SUB_PRE_JOB_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"

#define private public
#define protected public

class PluginSubPrepJobTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void PluginSubPrepJobTest::SetUp() {}

void PluginSubPrepJobTest::TearDown() {}

void PluginSubPrepJobTest::SetUpTestCase() {}

void PluginSubPrepJobTest::TearDownTestCase() {}

#endif