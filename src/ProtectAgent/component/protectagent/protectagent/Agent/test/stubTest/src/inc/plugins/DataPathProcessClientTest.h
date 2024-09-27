#ifndef __AGENT_DATAPATHPROCESS_CLIENT_TEST_H__
#define __AGENT_DATAPATHPROCESS_CLIENT_TEST_H__
#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "cunitpub/publicInc.h"

class DataPathProcessClientTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void DataPathProcessClientTest::SetUp() {}

void DataPathProcessClientTest::TearDown() {}

void DataPathProcessClientTest::SetUpTestCase() {}

void DataPathProcessClientTest::TearDownTestCase() {}

#endif /* _AGENT_SERVICE_PLUGIN_TEST_H_; */