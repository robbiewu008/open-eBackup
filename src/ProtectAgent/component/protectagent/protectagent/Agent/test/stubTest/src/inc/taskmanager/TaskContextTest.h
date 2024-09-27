/******************************************************************************
    Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.
    ******************************************************************************
    File Name     : TaskContext.h
    Version       : Initial Draft
    Author        : y00412658
    Created       :
    Last Modified :
    Description   : single instance, transfer task data
    History       :
******************************************************************************/
#ifndef _AGENT_TASKCONTEXT_TEST_H_
#define _AGENT_TASKCONTEXT_TEST_H_
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#define private public

class TaskContextTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void TaskContextTest::SetUp() {}

void TaskContextTest::TearDown() {}

void TaskContextTest::SetUpTestCase() {}

void TaskContextTest::TearDownTestCase() {}

#endif