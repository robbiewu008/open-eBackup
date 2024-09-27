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
#include "pluginfx/OraclePluginHandlerTest.h"
#include "pluginfx/OraclePluginHandler.h"
#include "servicecenter/thriftservice/detail/ThriftService.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "common/DB.h"
#include "common/ConfigXmlParse.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/ApplicationService.h"
#include "pluginfx/ExternalPluginManager.h"
using namespace thriftservice;
using namespace AppProtect;

namespace {
class ThriftClientStub : public thriftservice::IThriftClient {
public:
    virtual bool Start()
    {
        return true;
    }
    virtual bool Stop()
    {
        return true;
    }
    virtual std::shared_ptr<apache::thrift::protocol::TProtocol> GetTProtocol()
    {
        return nullptr;
    }
    virtual std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> GetSyncInfo()
    {
        return nullptr;
    }
};

class ApplicationServiceClientSub : public ApplicationServiceConcurrentClient {
public:
    ApplicationServiceClientSub(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ApplicationServiceConcurrentClient(prot, nullptr) {}

    void OracleCheckArchiveArea(ActionResult& _return, 
        const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList) override
    {
        _return.code = MP_SUCCESS;
    }
};


mp_void CLogger_Log_Stub(mp_void* pthis) {
    return;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_int32 QueryTableTest(mp_void* pthis, const mp_string& strSql, DbParamStream& dps,
    DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringFailed(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

std::shared_ptr<ExternalPlugin> StubGetPluginByRest(void* obj, const mp_string &appType)
{
    return std::make_shared<ExternalPlugin>("oracle", "test1", false, 59570);
}

std::shared_ptr<thriftservice::IThriftClient> StubGetPluginClient(void* obj)
{
    return std::make_shared<ThriftClientStub>();
}

static std::shared_ptr<ApplicationServiceClientSub> g_ApplicationServiceClient;
static std::shared_ptr<ApplicationServiceConcurrentClient> StubGetApplicationServiceClient(
    mp_void* pThis, const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient)
{
    if (g_ApplicationServiceClient.get() == nullptr) {
        g_ApplicationServiceClient = std::make_shared<ApplicationServiceClientSub>(nullptr);
    }
    return g_ApplicationServiceClient;
}
}

TEST_F(OraclePluginHandlerTest, OracleUpdateDBInfo)
{
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
        const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), CLogger_Log_Stub);

    Json::Value requestParam;
    mp_stub.set(ADDR(CDB, ExecSql), StubFailed);
    OraclePluginHandler::GetInstance().OracleUpdateDBInfo(requestParam);

    mp_stub.set(ADDR(CDB, ExecSql), StubSuccess);
    OraclePluginHandler::GetInstance().OracleUpdateDBInfo(requestParam);
}

TEST_F(OraclePluginHandlerTest, OracleQueryDBInfo)
{
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
        const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), CLogger_Log_Stub);

    std::vector<AppProtect::OracleDBInfo> vecDBInfo;
    mp_stub.set(ADDR(CDB, QueryTable), QueryTableTest);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFailed);
    OraclePluginHandler::GetInstance().OracleQueryDBInfo(vecDBInfo);
    EXPECT_EQ(1, vecDBInfo.size());
}

TEST_F(OraclePluginHandlerTest, OracleCheckArchiveArea)
{
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
        const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), CLogger_Log_Stub);
    mp_stub.set(ADDR(CDB, QueryTable), QueryTableTest);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFailed);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(OraclePluginHandler, GetApplicationServiceClient), StubGetApplicationServiceClient);

    OraclePluginHandler::GetInstance().OracleCheckArchiveArea();
}