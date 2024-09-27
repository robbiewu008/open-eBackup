#ifndef _AGENT_SERVICE_PLUGIN_TEST_H_
#define _AGENT_SERVICE_PLUGIN_TEST_H_
#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "cunitpub/publicInc.h"

class CRestActionMapTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void CRestActionMapTest::SetUp() {}

void CRestActionMapTest::TearDown() {}

void CRestActionMapTest::SetUpTestCase() {}

void CRestActionMapTest::TearDownTestCase() {}

#endif /* _AGENT_SERVICE_PLUGIN_TEST_H_; */

