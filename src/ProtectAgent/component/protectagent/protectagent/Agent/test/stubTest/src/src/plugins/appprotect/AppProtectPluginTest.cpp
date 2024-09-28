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
#include "plugins/appprotect/AppProtectPluginTest.h"
#include "plugins/appprotect/AppProtectPlugin.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "pluginfx/ExternalPluginManager.h"

namespace{
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
stub11.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_bool StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}
mp_string StubAppType(mp_void* pthis)
{
    return "";
}

std::shared_ptr<AppProtectService> AppProtectServiceGetInstance() {
    return nullptr;
}
}

TEST_F(CAppProtectPluginTest, Initialize)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;
    std::vector<mp_uint32> cmds;
    stub11.set(ADDR(CMpFile, DirExist), StubTrue);

    stub11.set(ADDR(ExternalPluginManager, Init), StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.Initialize(cmds));

    stub11.set(ADDR(ExternalPluginManager, Init), StubSuccess);
    stub11.set(ADDR(AppProtect::AppProtectJobHandler, Initialize), StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.Initialize(cmds));

    stub11.set(ADDR(ExternalPluginManager, Init), StubSuccess);
    stub11.set(ADDR(AppProtect::AppProtectJobHandler, Initialize), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.Initialize(cmds));
}

TEST_F(CAppProtectPluginTest, WakeUpJob)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = "/v1/tasks/386bf543-c45c-4ea1-8444-fc0b4bf66d54/notify";
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;

    stub11.set(&AppProtectService::WakeUpJob, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));

    stub11.set(&AppProtectService::WakeUpJob, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
}

TEST_F(CAppProtectPluginTest, AbortJob)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = "/v1/tasks/386bf543-c45c-4ea1-8444-fc0b4bf66d54/abort";
    req.m_httpReq.m_strMethod = REST_URL_METHOD_PUT;

    stub11.set(&AppProtectService::AbortJob, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.AbortJob(req, rsp));

    stub11.set(&AppProtectService::AbortJob, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.AbortJob(req, rsp));
}

/*
* 用例名称：调用/v1/tasks/resource
* 前置条件：1、mock ExternalPluginManager::QueryPluginResource
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, PluginResourceV1)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_APPPROTECT_RESOURCE_V1;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_GET;
    stub11.set(&ExternalPluginManager::QueryPluginResource, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&ExternalPluginManager::QueryPluginResource, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
    EXPECT_EQ(ERR_OPERATION_FAILED, rsp.GetJsonValueRef()["code"].asInt());
}

/*
* 用例名称：调用/v1/tasks/detail
* 前置条件：1、mock ExternalPluginManager::QueryPluginDetail
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, PluginDetailV1)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_APPPROTECT_DETAIL_V1;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    stub11.set(&ExternalPluginManager::QueryPluginDetail, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&ExternalPluginManager::QueryPluginDetail, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
    EXPECT_EQ(ERR_OPERATION_FAILED, rsp.GetJsonValueRef()["code"].asInt());
}

/*
* 用例名称：调用/v1/tasks/check
* 前置条件：1、mock ExternalPluginManager::CheckPlugin
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, PluginCheckV1)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_APPPROTECT_CHECK_V1;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    stub11.set(&ExternalPluginManager::CheckPlugin, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&ExternalPluginManager::CheckPlugin, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
    EXPECT_EQ(ERR_OPERATION_FAILED, rsp.GetJsonValueRef()["code"].asInt());
}

/*
* 用例名称：调用/v1/tasks/cluster
* 前置条件：1、mock ExternalPluginManager::QueryRemoteCluster
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, PluginClusterV1)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_APPPROTECT_CLUSTER_V1;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    stub11.set(&ExternalPluginManager::QueryRemoteCluster, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&ExternalPluginManager::QueryRemoteCluster, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
    EXPECT_EQ(ERR_OPERATION_FAILED, rsp.GetJsonValueRef()["code"].asInt());
}

/*
* 用例名称：调用/v2/tasks/detail
* 前置条件：1、mock ExternalPluginManager::QueryPluginDetailV2
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, PluginDetailV2)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_APPPROTECT_DETAIL_V2;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    stub11.set(&ExternalPluginManager::QueryPluginDetailV2, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&ExternalPluginManager::QueryPluginDetailV2, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
    EXPECT_EQ(ERR_OPERATION_FAILED, rsp.GetJsonValueRef()["code"].asInt());
}

/*
* 用例名称：调用/v1/tasks/taskID/sanclient
* 前置条件：1、mock 调用函数
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, SanclientJob)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = "/v1/tasks/f7c624b3-18e3-4f58-8980-f9e5a27872cd/sanclient";
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    stub11.set(&AppProtectPlugin::SanclientPrepareJob, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&AppProtectPlugin::SanclientPrepareJob, StubFailed);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
}

/*
* 用例名称：调用/v1/tasks/taskID/queryluninfo
* 前置条件：1、mock 调用函数
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, SanclientJobForUbc)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = "/v1/tasks/f7c624b3-18e3-4f58-8980-f9e5a27872cd/queryluninfo";
    req.m_httpReq.m_strMethod = REST_URL_METHOD_GET;
    stub11.set(&AppProtectService::SanclientJobForUbc, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
}


/*
* 用例名称：调用/v1/tasks/taskID/sanclientclean
* 前置条件：1、mock 调用函数
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, SanclientCleanJob)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;

    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = "/v1/tasks/f7c624b3-18e3-4f58-8980-f9e5a27872cd/sanclientclean";
    req.m_httpReq.m_strMethod = REST_URL_METHOD_POST;
    req.m_msgBody.m_msgJsonData["task_id"] = "qqqqqqqqqqqqqq";
    stub11.set(&AppProtectService::CleanEnv, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.DoAction(req, rsp));
    stub11.set(&AppProtectService::CleanEnv, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.DoAction(req, rsp));
}


/*
* 用例名称：调用/v1/tasks/taskID/sanclientclean
* 前置条件：1、mock 调用函数
* check点： 1、mock成功，返回MP_SUCCESS
                    2、mock失败，返回MP_FAILED，检查返回值
*/
TEST_F(CAppProtectPluginTest, SanclientPrepareJob)
{
    DoGetJsonStringTest();
    AppProtectPlugin plugObj;
    Json::Value jvReq;
    jvReq["taskID"] = "qqqqqqqqqqqqqq";
    jvReq["sanclient"]["ip"] = "11.11.11.11";
    jvReq["sanclient"]["openLanFreeSwitch"] = "1";
    mp_string tmp0 = "0000";
    mp_string tmp1 = "1111";
    mp_string tmp2 = "2222";
    jvReq["agentWwpns"].append(std::move(tmp0));
    jvReq["sanclient"]["sanClientWwpns"].append(std::move(tmp1));
    jvReq["sanclient"]["sanClientWwpns"].append(std::move(tmp2));

    stub11.set(&AppProtectService::EnvCheck, StubSuccess);
    stub11.set(&AppProtectService::SanclientMount, StubSuccess);
    stub11.set(&AppProtectService::CreateLun, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, plugObj.SanclientPrepareJob(jvReq));

    stub11.set(&AppProtectService::EnvCheck, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.SanclientPrepareJob(jvReq));

    stub11.set(&AppProtectService::EnvCheck, StubSuccess);
    stub11.set(&AppProtectService::SanclientMount, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.SanclientPrepareJob(jvReq));

    stub11.set(&AppProtectService::EnvCheck, StubSuccess);
    stub11.set(&AppProtectService::SanclientMount, StubSuccess);
    stub11.set(&AppProtectService::CreateLun, StubFailed);
    EXPECT_EQ(MP_FAILED, plugObj.SanclientPrepareJob(jvReq));

    stub11.set(&AppProtectService::GetInstance, AppProtectServiceGetInstance);
    EXPECT_EQ(MP_FAILED, plugObj.SanclientPrepareJob(jvReq));
}
