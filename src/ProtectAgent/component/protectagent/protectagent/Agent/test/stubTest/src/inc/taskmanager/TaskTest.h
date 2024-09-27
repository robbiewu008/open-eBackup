/******************************************************************************
    Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.
    ******************************************************************************
    File Name     : Task.h
    Version       : Initial Draft
    Author        :
    Created       :
    Last Modified :
    Description   : basic class of all pluginXTasks
    History       :
******************************************************************************/
#ifndef _AGENT_TASK_TEST_H_
#define _AGENT_TASK_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#define private public
#define protected public

class TaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskTest::SetUp() {}

void TaskTest::TearDown() {}

void TaskTest::SetUpTestCase() {}

void TaskTest::TearDownTestCase() {}

#endif