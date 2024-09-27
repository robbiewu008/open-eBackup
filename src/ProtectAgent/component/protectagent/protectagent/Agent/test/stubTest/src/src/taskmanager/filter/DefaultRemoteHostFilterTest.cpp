/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "taskmanager/filter/DefaultRemoteHostFilterTest.h"
#include "taskmanager/filter/DefaultRemoteHostFilter.h"
#include "taskmanager/filter/RemoteHostFilterFactory.h"
#include "taskmanager/externaljob/Job.h"

namespace{

#define MOCK_JOB_JSON                           \
"{                                              \
    \"taskId\": \"taskId\",                     \
    \"taskName\": \"taskName\",                 \
    \"taskType\": 1,                            \
    \"copies\":                                 \
    [                                           \
        {                                       \
            \"id\": \"id\",                     \
            \"repositories\":                   \
            [                                   \
                {                               \
                \"remotePath\": \"path\",       \
                \"remoteHost\":                 \
                [                               \
                    {                           \
                        \"ip\": \"1.1.1.1\",    \
                        \"portType\": 8         \
                    },                          \
                    {                           \
                        \"ip\": \"2.2.2.2\",    \
                        \"portType\": 9         \
                    }                           \
                ]                               \
                }                               \
            ]                                   \
        }                                       \
    ],                                          \
    \"appInfo\":                                \
    {                                           \
        \"type\": \"Fileset\",                  \
        \"subType\": \"Fileset\",               \
        \"uuid\": \"uuid\",                     \
        \"name\": \"name\",                     \
        \"extendInfo\":                         \
        {                                       \
            \"lanType\": \"0\"                  \
        }                                       \
    },                                          \
    \"repositories\":                           \
    [                                           \
        {                                       \
        \"remotePath\": \"path\",               \
        \"remoteHost\":                         \
        [                                       \
            {                                   \
                \"ip\": \"1.1.1.1\",            \
                \"portType\": 8                 \
            },                                  \
            {                                   \
                \"ip\": \"2.2.2.2\",            \
                \"portType\": 9                 \
            }                                   \
        ]                                       \
        }                                       \
    ]                                           \
}"
}

using namespace AppProtect;

void DefaultRemoteHostFilterTest::SetUp()
{
    filter = std::dynamic_pointer_cast<DefaultRemoteHostFilter>(RemoteHostFilterFactory::CreateFilter(PORT_TYPE_FILTER));
    jsonReader = Json::Reader();
    jobData = {"pluginName", "mainID", "subID", Json::Value(), MainJobType::BACKUP_JOB};
}

void DefaultRemoteHostFilterTest::TearDown()
{
}

void DefaultRemoteHostFilterTest::SetUpTestCase()
{
}

void DefaultRemoteHostFilterTest::TearDownTestCase()
{
}

mp_int32 Stub_GetLANTypeSucc(void* obj, Json::Value& jobParam, mp_int32& lanType)
{
    lanType = APPINFO_LAN_TYPE_VLAN;
    return MP_SUCCESS;
}

mp_int32 Stub_GetLANTypeFailed(void* obj, Json::Value& jobParam, mp_int32& lanType)
{
    return MP_FAILED;
}


/*
* 用例名称：从任务参数获取lanType失败
* 前置条件：任务数据中无端口类型字段
* check点：获取端口类型失败，返回值MP_FALSE
*/
TEST_F(DefaultRemoteHostFilterTest, GetLANTypeTestFail)
{
    mp_int32 lanType;
    Json::Value param;
    EXPECT_EQ(filter->GetLANType(param, lanType), MP_FALSE);
}

/*
* 用例名称：从任务参数获取lanType成功
* 前置条件：任务数据中下发端口类型字段
* check点：获取端口类型成功，返回值MP_SUCCESS，且返回消息中有lanType类型
*/
TEST_F(DefaultRemoteHostFilterTest, GetLANTypeTestSucc)
{
    Json::Value root;
    jsonReader.parse(MOCK_JOB_JSON, root, false);
    mp_int32 lanType;
    EXPECT_EQ(filter->GetLANType(root, lanType), MP_FALSE);
    EXPECT_EQ(lanType, APPINFO_LAN_TYPE_VLAN);
}

/*
* 用例名称：过滤逻辑端口失败
* 前置条件：1.为内置代理isInner=true。2.为外置代理isInner=false，参数未下发lanType
* check点：返回值为MP_FAILED
*/
TEST_F(DefaultRemoteHostFilterTest, DoFilterTestFail)
{
    mp_bool isInner = true;
    EXPECT_EQ(filter->DoFilter(jobData, isInner), MP_FAILED);
    isInner = false;
    stub.set(&DefaultRemoteHostFilter::GetLANType, Stub_GetLANTypeFailed);
    EXPECT_EQ(filter->DoFilter(jobData, isInner), MP_FAILED);
    stub.reset(&DefaultRemoteHostFilter::GetLANType);
}

/*
* 用例名称：过滤逻辑端口成功VLAN
* 前置条件：1.外置代理，isInner=false。2.GetLANType成功。3.lanType=0(VLAN)
* check点：返回值为MP_SUCCESS
*/
TEST_F(DefaultRemoteHostFilterTest, DoFilterTestSucc)
{
    mp_bool isInner = false;
    Json::Value root;
    jsonReader.parse(MOCK_JOB_JSON, root, false);
    jobData.param = root;
    stub.set(&DefaultRemoteHostFilter::GetLANType, Stub_GetLANTypeSucc);
    EXPECT_EQ(filter->DoFilter(jobData, isInner), MP_SUCCESS);
    EXPECT_EQ(jobData.param[JobParamKey::REPOSITORIES][0][JobParamKey::REMOTE_HOST].size(), 1);
    EXPECT_EQ(jobData.param[JobParamKey::REPOSITORIES][0][JobParamKey::REMOTE_HOST][0][JobParamKey::PORT_TYPE], LOGIC_PORT_TYPE_VLAN);
    EXPECT_EQ(jobData.param[JobParamKey::COPIES][0][JobParamKey::REPOSITORIES][0][JobParamKey::REMOTE_HOST].size(), 1);
    EXPECT_EQ(jobData.param[JobParamKey::COPIES][0][JobParamKey::REPOSITORIES][0][JobParamKey::REMOTE_HOST][0][JobParamKey::PORT_TYPE], LOGIC_PORT_TYPE_VLAN);
    stub.reset(&DefaultRemoteHostFilter::GetLANType);
}