/******************************************************************************
    Copyright (C), 2001-2021, Huawei Tech. Co., Ltd.
    ******************************************************************************
    File Name     : PluginMainJobTest.h
    Version       : Initial Draft
    Author        : w00510599
    Created       :
    Last Modified :
    Description   : single instance, transfer task data
    History       :
******************************************************************************/
#ifndef _APP_PROTECT_JOB_HANDLER_TEST_H_
#define _APP_PROTECT_JOB_HANDLER_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/Log.h"

#define private public
#define protected public

class PluginMainJobTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void PluginMainJobTest::SetUp() {}

void PluginMainJobTest::TearDown() {}

void PluginMainJobTest::SetUpTestCase() {}

void PluginMainJobTest::TearDownTestCase() {}

#endif