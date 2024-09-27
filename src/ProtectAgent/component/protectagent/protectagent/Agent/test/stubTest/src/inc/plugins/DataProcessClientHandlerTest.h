#ifndef __AGENT_VMWARENATIVE_DATAPROCESS_HANDLER_TEST_H__
#define __AGENT_VMWARENATIVE_DATAPROCESS_HANDLER_TEST_H__
#include <vector>
#include <algorithm>
#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "cunitpub/publicInc.h"

class DataProcessClientHandlerTest : public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub11;
};

void DataProcessClientHandlerTest::SetUp() {}

void DataProcessClientHandlerTest::TearDown() {}

void DataProcessClientHandlerTest::SetUpTestCase() {}

void DataProcessClientHandlerTest::TearDownTestCase() {}

#endif