#ifndef __EXTERNAL_PLUGIN_PARSE_H__
#define __EXTERNAL_PLUGIN_PARSE_H__

#define private public
#define protected public

#include "pluginfx/ExternalPluginParse.h"
#include "securec.h"
#include "common/ErrorCode.h"
#include "gtest/gtest.h"
#include "stub.h"
class ExternalPluginParseTest : public testing::Test {
public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }
};

#endif