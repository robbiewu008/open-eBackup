 /**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginTest.h
 * @brief  The implemention about ExternalPluginTest.h
 * @version 1.1.0.0
 * @date 2021-12-17
 * @author mwx1011302
 */
#ifndef _EXTERNAL_PLUGIN_TEST_H
#define _EXTERNAL_PLUGIN_TEST_H

#define private public
#define protected public

#include "gtest/gtest.h"
class ExternalPluginTest : public testing::Test {
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