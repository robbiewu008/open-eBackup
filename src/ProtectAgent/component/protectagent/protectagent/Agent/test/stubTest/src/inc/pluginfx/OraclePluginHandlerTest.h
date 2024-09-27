#ifndef _ORACLE_PLUGIN_HANDLER_TEST_H
#define _ORACLE_PLUGIN_HANDLER_TEST_H

#define private public
#define protected public

#include "gtest/gtest.h"
#include "stub.h"

class OraclePluginHandlerTest : public testing::Test {
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