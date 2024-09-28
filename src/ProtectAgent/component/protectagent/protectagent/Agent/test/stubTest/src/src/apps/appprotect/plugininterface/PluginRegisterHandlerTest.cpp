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
#include "apps/appprotect/plugininterface/PluginRegisterHandlerTest.h"
#include "apps/appprotect/plugininterface/PluginRegisterHandler.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "common/Log.h"
#include "stub.h"
#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/messageservice/include/IObserver.h"


using namespace AppProtect;
using namespace servicecenter;
using namespace messageservice;
using namespace thriftservice::detail;
namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}
}

mp_void PluginRegisterHandlerTestLogVoid(mp_void* pthis)
{
    return;
}

void StubRegisterObserverSuccess1(messageservice::EVENT_TYPE type, std::shared_ptr<IObserver> observer)
{
    return;
}

void StubInitMonitorThread1(mp_void *pThis)
{
}

mp_int32 StubUpdatePluginInfoSuccess1(const mp_string &pluginName, const ApplicationPlugin &pluginInfo)
{
    return MP_SUCCESS;
}

mp_int32 StubUpdatePluginInfoFail(const mp_string &pluginName, const ApplicationPlugin &pluginInfo)
{
    return MP_FAILED;
}

mp_int32 StubUpdatePluginStatusSuccess1(const mp_string &pluginName, EX_PLUGIN_STATUS status)
{
    return MP_SUCCESS;
}

mp_int32 StubUpdatePluginStatusFail(const mp_string &pluginName, EX_PLUGIN_STATUS status)
{
    return MP_FAILED;
}

mp_void* StubMonPluginStatusThread1(mp_void *pThis)
{
    return nullptr;
}

void StubRegisterObserver(messageservice::EVENT_TYPE type, std::shared_ptr<IObserver> observer)
{
}

bool StubRegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor)
{
    return MP_FALSE;
}

/*
*用例名称：插件注册成功
*前置条件：插件上报的插件是由代理拉起
*check点：代理能成功拉起插件，插件完成注册
*/
TEST_F(PluginRegisterHandlerTest, registerSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginInfo), StubUpdatePluginInfoSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubUpdatePluginStatusSuccess1);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->RegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, 0);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：插件注册失败，因为插件上报的信息错误
*前置条件：插件上报的插件是由代理拉起
*check点：当插件注册信息不全时，注册失败
*/
TEST_F(PluginRegisterHandlerTest, registerFailForWrongParam) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginInfo), StubUpdatePluginInfoSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubUpdatePluginStatusSuccess1);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->RegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, MP_FAILED);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：插件注册失败，因为更新插件信息失败
*前置条件：插件上报的插件是由代理拉起
*check点：当更新插件信息失败时，注册失败
*/
TEST_F(PluginRegisterHandlerTest, registerFailForUpdateInfoFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginInfo), StubUpdatePluginInfoFail);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->RegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, MP_FAILED);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：插件注册失败，因为更新插件状态失败
*前置条件：插件上报的插件是由代理拉起
*check点：当更新插件状态失败时，注册失败
*/
TEST_F(PluginRegisterHandlerTest, registerFailForUpdateStatusFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginInfo), StubUpdatePluginInfoSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubUpdatePluginStatusFail);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->RegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, MP_FAILED);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：插件去注册成功
*前置条件：插件去注册信息正确
*check点：插件能去注册成功
*/
TEST_F(PluginRegisterHandlerTest, unregisterSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubUpdatePluginStatusSuccess1);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->UnRegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, 0);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：插件去注册失败，因为更新插件状态失败
*前置条件：插件去注册信息正确
*check点：当更新插件状态失败，插件去注册失败
*/
TEST_F(PluginRegisterHandlerTest, unregisterFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubUpdatePluginStatusFail);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    plugin.processId = 123;
    handler->UnRegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, MP_FAILED);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}

/*
*用例名称：去初始化
*前置条件：无
*check点：永久成功
*/
TEST_F(PluginRegisterHandlerTest, uninitializeSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    EXPECT_EQ(handler->Uninitailize(), true);
}

/*
*用例名称：初始化
*前置条件：无
*check点：永久成功
*/
TEST_F(PluginRegisterHandlerTest, initializeSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubRegisterObserver);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    EXPECT_EQ(handler->Initailize(), true);
}

/*
*用例名称：更新成功
*前置条件：无
*check点：当通知事件指针不为空
*/
TEST_F(PluginRegisterHandlerTest, UpdateSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubRegisterObserver);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));

    auto tmpThriftService = ServiceFactory::GetInstance()->GetService<ThriftService>("IThriftService");
    std::shared_ptr<thriftservice::IThriftServer> tmpThriftServiceNew =
        std::dynamic_pointer_cast<thriftservice::IThriftServer>(tmpThriftService);
    std::shared_ptr<messageservice::RpcPublishEvent> event =
        std::make_shared<messageservice::RpcPublishEvent>(tmpThriftServiceNew);
    handler->Update(event);
}

/*
*用例名称：更新失败，因为thrift服务端指针为空
*前置条件：无
*check点：当thrift服务端指针为空，更新失败
*/
TEST_F(PluginRegisterHandlerTest, UpdateFailForNullThrift) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, RegisterObserver), StubRegisterObserverSuccess1);
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread1);
    mp_stub.set(ADDR(ExternalPluginManager, InitMonitorThread), StubInitMonitorThread1);
    mp_stub.set(ADDR(ExternalPluginManager, UpdatePluginStatus), StubRegisterObserver);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));

    std::shared_ptr<thriftservice::IThriftServer> tmpThriftServiceNew;
    std::shared_ptr<messageservice::RpcPublishEvent> event =
        std::make_shared<messageservice::RpcPublishEvent>(tmpThriftServiceNew);
    handler->Update(event);
}

TEST_F(PluginRegisterHandlerTest, RegisterPluginRegistrationParam) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32,
        const mp_string&, const mp_string&, ...))ADDR(CLogger, Log), PluginRegisterHandlerTestLogVoid);
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->Register<PluginRegisterHandler>("PluginRegisterService");
    std::shared_ptr<IService> tempService =
        ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    std::shared_ptr<PluginRegisterHandler> handler(std::dynamic_pointer_cast<PluginRegisterHandler>(tempService));
    ActionResult ret;
    ApplicationPlugin plugin;
    plugin.endPoint = "127.0.0.1";
    plugin.name = "NasShare";
    plugin.port = 59610;
    handler->RegisterPlugin(ret, plugin);
    EXPECT_EQ(ret.code, MP_FAILED);
    ServiceFactory::GetInstance()->Unregister("PluginRegisterService");
}
