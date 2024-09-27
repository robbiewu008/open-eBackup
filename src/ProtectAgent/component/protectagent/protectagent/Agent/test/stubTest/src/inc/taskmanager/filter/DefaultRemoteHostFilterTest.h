/**
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* @file DefaultRemoteHostFilterTest.h
* @brief LLT of DefaultRemoteHostFilter
* @version 0.1
* @date 2023-07-07
* @author wangyunlong w30045225
*/
#ifndef _AGENT_DEFAULT_REMOTE_HOST_FILTER_TEST_H_
#define _AGENT_DEFAULT_REMOTE_HOST_FILTER_TEST_H_
#include <vector>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "taskmanager/filter/DefaultRemoteHostFilter.h"


class DefaultRemoteHostFilterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<DefaultRemoteHostFilter> filter;
    Stub stub;
    Json::Reader jsonReader;
    PluginJobData jobData;
};
#endif