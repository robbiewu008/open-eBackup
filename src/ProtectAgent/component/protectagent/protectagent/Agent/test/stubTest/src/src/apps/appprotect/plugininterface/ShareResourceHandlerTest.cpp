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
#include "apps/appprotect/plugininterface/ShareResourceHandlerTest.h"
#include "apps/appprotect/plugininterface/ShareResourceHandler.h"
#include "message/curlclient/DmeRestClient.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "common/Log.h"
#include "stub.h"
#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/messageservice/include/IObserver.h"
#include "servicecenter/thriftservice/detail/ThriftServer.h"
#include "message/curlclient/RestClientCommon.h"
#include <taskmanager/externaljob/AppProtectJobHandler.h>
#include "common/ConfigXmlParse.h"
#include "common/ErrorCode.h"
#include "host/host.h"

using namespace AppProtect;
using namespace servicecenter;
using namespace messageservice;
using namespace thriftservice::detail;
namespace {
mp_void CLogger_Log_Stub(mp_void* pthis) {
    return;
}
}

mp_void StubShareResourceTestLogVoid(mp_void* pthis){
    return;
}

void StubRegisterObserverSuccess(messageservice::EVENT_TYPE type, std::shared_ptr<IObserver> observer)
{
    return;
}

bool StuThriftServerStart(void *ptr)
{
    return true;
}

mp_int32 StubGetConfigValueInt32Success(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 0;
    return MP_SUCCESS;
}

mp_int32 StubUpdateSecureInfoSuccess()
{
    return MP_SUCCESS;
}

mp_void* StubMonPluginStatusThread(mp_void *pThis)
{
    return nullptr;
}

void StubInitMonitorThread(mp_void *pThis)
{

}

mp_int32 StubSendRequestSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value value;
    value["errorCode"] = "0";
    httpResponse.body = value.toStyledString();
    return MP_SUCCESS;
}

mp_int32 StubSendRequestFail(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value value;
    value["errorCode"] = "200";
    httpResponse.body = value.toStyledString();
    return MP_FAILED;
}

mp_int32 StubSendRequestQuery(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    Json::Value value, resource;
    resource["scope"] = 1;
    resource["scopeKey"] = "111";
    resource["resourceKey"] = "res";
    resource["resourceValue"] = "val";
    value["resource"] = resource;
    httpResponse.body = value.toStyledString();
    return MP_SUCCESS;
}

mp_int32 StubGetHostSNSuccess(void *obj, mp_string& strSN)
{
    strSN = "123456";
    return MP_SUCCESS;
}

mp_uint32 StubGetHttpStatusCodeScOk()
{
    return SC_OK;
}

mp_int32 StubFailed(void *obj)
{
    return MP_FAILED;
}

mp_int32 StubConvertStrToRspMsgs(const std::string &rspstr, RestClientCommon::RspMsg &rspSt)
{
    rspSt.errorCode = "0";
    return MP_FAILED;
}

mp_int32 StubSuccess(void *obj)
{
    return MP_SUCCESS;
}

mp_int32 StubConvertStringtoJson(const mp_string& rawBuffer, Json::Value& jsValue)
{
    jsValue["resource"]["scope"] = 3;
    jsValue["lockNum"] = 1;
    return MP_SUCCESS;
}

mp_bool StubFalse(void *obj)
{
    return MP_FALSE;
}

mp_bool StubTrue(void *obj)
{
    return MP_TRUE;
}

mp_int32 StubGetUbcIpsByMainJobId(mp_void* pThis, const mp_string mainJobId, std::vector<mp_string>& ubcIps)
{
    return MP_SUCCESS;
}

/*
*用例名称：创建节点级别共享资源失败
*前置条件：无法获取到节点ID
*check点：创建节点级别共享资源会因为无法获取到节点ID失败
*/
TEST_F(ShareResourceHandlerTest, createNodeShareResourceFailed) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(DmeRestClient, UpdateSecureInfo), StubUpdateSecureInfoSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    ServiceFactory::GetInstance()->Register<ShareResourceHandler>("ShareResourceService");
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    resource.scope = ResourceScope::type::SINGLE_NODE;
    handler->CreateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, 200);
    ServiceFactory::GetInstance()->Unregister("ShareResourceService");
}
/*
*用例名称：创建共享资源成功
*前置条件：与DME可连通，且资源不存在
*check点：创建共享资源成功
*/
TEST_F(ShareResourceHandlerTest, createShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(DmeRestClient, UpdateSecureInfo), StubUpdateSecureInfoSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    ServiceFactory::GetInstance()->Register<ShareResourceHandler>("ShareResourceService");
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->CreateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}
/*
*用例名称：创建节点级别共享资源成功
*前置条件：与DME可连通，且资源不存在
*check点：创建节点级别共享资源成功
*/
TEST_F(ShareResourceHandlerTest, createNodeShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(DmeRestClient, UpdateSecureInfo), StubUpdateSecureInfoSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    resource.scope = ResourceScope::type::SINGLE_NODE;
    handler->CreateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}
/*
*用例名称：创建共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源已存在
*check点：创建共享资源失败
*/
TEST_F(ShareResourceHandlerTest, createShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->CreateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, RPC_ACTION_EXECUTIVE_INTERNAL_ERROR);
}

/*
*用例名称：查询共享资源成功
*前置条件：与DME可连通，资源存在
*check点：查询共享资源成功
*/
TEST_F(ShareResourceHandlerTest, queryShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestQuery);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ResourceStatus status;
    Resource resource;
    handler->QueryResource(status, resource, "mainId");
}

/*
*用例名称：查询共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源不存在
*check点：查询共享资源失败
*/
TEST_F(ShareResourceHandlerTest, queryShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ResourceStatus status;
    Resource resource;
    try {
        handler->QueryResource(status, resource, "mainId");
    } catch (...) {
    }
}

/*
*用例名称：更新共享资源成功
*前置条件：与DME可连通，资源存在
*check点：更新共享资源成功
*/
TEST_F(ShareResourceHandlerTest, updateShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->UpdateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}

/*
*用例名称：更新共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源不存在
*check点：更新共享资源失败
*/
TEST_F(ShareResourceHandlerTest, updateShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->UpdateResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, RPC_ACTION_EXECUTIVE_INTERNAL_ERROR);
}

/*
*用例名称：删除共享资源成功
*前置条件：与DME可连通，资源存在
*check点：删除共享资源成功
*/
TEST_F(ShareResourceHandlerTest, deleteShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->DeleteResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}

/*
*用例名称：删除共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源不存在
*check点：删除共享资源失败
*/
TEST_F(ShareResourceHandlerTest, deleteShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->DeleteResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, RPC_ACTION_EXECUTIVE_INTERNAL_ERROR);
}

/*
*用例名称：锁定共享资源成功
*前置条件：与DME可连通，资源存在
*check点：锁定共享资源成功
*/
TEST_F(ShareResourceHandlerTest, lockShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    int32_t timeout = 30;
    handler->LockResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}

/*
*用例名称：锁定共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源不存在 3. 资源已被锁定
*check点：锁定共享资源失败
*/
TEST_F(ShareResourceHandlerTest, lockShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler = (std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    int32_t timeout = 30;
    handler->LockResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, RPC_ACTION_EXECUTIVE_INTERNAL_ERROR);
}

/*
*用例名称：解锁共享资源成功
*前置条件：与DME可连通，资源已锁定
*check点：解锁共享资源成功
*/
TEST_F(ShareResourceHandlerTest, unlockShareResourceSucess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->UnLockResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, MP_SUCCESS);
}

/*
*用例名称：解锁共享资源失败
*前置条件：以下情况可能会失败：1. 与DME不可连通 2. 资源不存在 3. 资源未被锁定过
*check点：锁定共享资源失败
*/
TEST_F(ShareResourceHandlerTest, unlockShareResourceFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFail);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    ActionResult ret;
    Resource resource;
    handler->UnLockResource(ret, resource, "mainId");
    EXPECT_EQ(ret.code, RPC_ACTION_EXECUTIVE_INTERNAL_ERROR);
}

TEST_F(ShareResourceHandlerTest, CheckResourceJsonValidTest) {
    Stub mp_stub;
    Json::Value resourceInfo;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));

    mp_bool bRet = handler->CheckResourceJsonValid(resourceInfo);
    EXPECT_EQ(false, bRet);
    resourceInfo["scope"] = "scope";
    bRet = handler->CheckResourceJsonValid(resourceInfo);
    EXPECT_EQ(false, bRet);
}

TEST_F(ShareResourceHandlerTest, UpdateResourceTest) {
    Stub mp_stub;
    mp_int32 entryCount = 0;
    ActionResult _return;
    Resource resource;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ShareResourceHandler, SwitchResourceToJson), StubFailed);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    handler->UpdateResource(_return, resource, "mainId");
    EXPECT_EQ(0 , entryCount);
}

TEST_F(ShareResourceHandlerTest, DeleteResourceTest) {
    Stub mp_stub;
    mp_int32 entryCount = 0;
    ActionResult _return;
    Resource resource;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ShareResourceHandler, AssignScopeKey), StubFailed);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    handler->DeleteResource(_return, resource, "mainId");
    EXPECT_EQ(0 , entryCount);
}

TEST_F(ShareResourceHandlerTest, LockResourceTest) {
    Stub mp_stub;
    mp_int32 entryCount = 0;
    ActionResult _return;
    Resource resource;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(ADDR(ShareResourceHandler, SwitchResourceToJson), StubFailed);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    handler->LockResource(_return, resource, "mainId");
    handler->UnLockResource(_return, resource, "mainId");
    EXPECT_EQ(0 , entryCount);
}

TEST_F(ShareResourceHandlerTest, CommonSendRequestHandleTest) {
    Stub mp_stub;
    ActionResult _return;
    DmeRestClient::HttpReqParam req;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubFalse);
    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubFailed);
    handler->CommonSendRequestHandle(_return, req);
    EXPECT_EQ(RPC_ACTION_EXECUTIVE_INTERNAL_ERROR , _return.code);

    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubFalse);
    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubConvertStrToRspMsgs);
    handler->CommonSendRequestHandle(_return, req);
    EXPECT_EQ(RPC_ACTION_EXECUTIVE_INTERNAL_ERROR , _return.code);
}

TEST_F(ShareResourceHandlerTest, QueryResourceTest) {
    Stub mp_stub;
    ResourceStatus _return;
    Resource resource;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    mp_stub.set(ADDR(ShareResourceHandler, AssignScopeKey), StubFailed);
    EXPECT_THROW(handler->QueryResource(_return, resource, "mainId"), AppProtectFrameworkException);

    mp_stub.set(ADDR(ShareResourceHandler, AssignScopeKey), StubSuccess);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSuccess);
    mp_stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubFailed);
    EXPECT_THROW(handler->QueryResource(_return, resource, "mainId"), AppProtectFrameworkException);

    mp_stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubConvertStringtoJson);
    mp_stub.set(ADDR(ShareResourceHandler, CheckResourceJsonValid), StubFalse);
    EXPECT_THROW(handler->QueryResource(_return, resource, "mainId"), AppProtectFrameworkException);

    mp_stub.set(ADDR(ShareResourceHandler, CheckResourceJsonValid), StubTrue);
    EXPECT_THROW(handler->QueryResource(_return, resource, "mainId"), AppProtectFrameworkException);
}

TEST_F(ShareResourceHandlerTest, UpdateSuccess) {
    Stub mp_stub;
    mp_int32 entryCount = 0;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    auto tmpThriftService = ServiceFactory::GetInstance()->GetService<ThriftService>("IThriftService");
    std::shared_ptr<thriftservice::IThriftServer> tmpThriftServiceNew =
        std::dynamic_pointer_cast<thriftservice::IThriftServer>(tmpThriftService);
    std::shared_ptr<messageservice::RpcPublishEvent> event =
        std::make_shared<messageservice::RpcPublishEvent>(tmpThriftServiceNew);
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    handler->Update(event);
}

TEST_F(ShareResourceHandlerTest, UpdateFailForNullThrift) {
    Stub mp_stub;
    mp_int32 entryCount = 0;
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    std::shared_ptr<ShareResourceHandler> handler(std::dynamic_pointer_cast<ShareResourceHandler>(tempService));
    auto tmpThriftService = ServiceFactory::GetInstance()->GetService<ThriftService>("IThriftService");
    std::shared_ptr<thriftservice::IThriftServer> tmpThriftServiceNew;
    std::shared_ptr<messageservice::RpcPublishEvent> event =
        std::make_shared<messageservice::RpcPublishEvent>(tmpThriftServiceNew);
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubShareResourceTestLogVoid);
    handler->Update(event);
}

