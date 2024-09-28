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
#include "apps/appprotect/AppProtectServiceTest.h"
#include "taskmanager/AppProtectJobHandlerTest.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "host/host.h"
#include "common/Ip.h"
#include "securecom/RootCaller.h"
#include "common/Utils.h"

void AppProtectServiceTest::SetUp()
{}

void AppProtectServiceTest::TearDown()
{}

void AppProtectServiceTest::SetUpTestCase()
{}

void AppProtectServiceTest::TearDownTestCase()
{}

mp_int32 StubAbortJob(const std::string &mainTaskId, const std::string &subtaskId)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckIsDoradoEnvironmentSuccess(void *obj, mp_bool& isDorado)
{
    isDorado = true;
    return MP_SUCCESS;
}

mp_int32 StubMountNasFileSystem(void* obj, AppProtect::MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, 
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const AppProtect::MountPermission &permit)
{
    successMountPath.push_back("/mnt/databackup/pluginName/mainJobID/data/Share1/192.168.145.161");
    return MP_SUCCESS;
}


std::smatch StubGetTaskIdFromUrlEmpty(const mp_string& url){
    std::smatch match;
    return match;
}

AppProtect::AppProtectJobHandler* StubGetInstanceNull(){
    return nullptr;
}

mp_int32 StubTestCheckIsDoradoEnvironmentSuccess(mp_bool& isDorado)
{
    isDorado = true;
    return MP_SUCCESS;
}
mp_int32 StubGetDoraDoLanNet(void *obj, mp_string& net)
{
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 StubInitFailed(void* obj)
{
    return MP_FAILED;
}

static int32_t g_countStart = 0;
class ThriftClientStub : public thriftservice::IThriftClient {
public:
    virtual bool Start()
    {
        return --g_countStart == 0;
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

std::shared_ptr<ExternalPlugin> StubGetPluginByRest(void* obj, const mp_string &appType)
{
    return std::make_shared<ExternalPlugin>("HDFS", "test1", false, 59570);
}

std::shared_ptr<ExternalPlugin> StubGetPluginByRest_Null(void* obj, const mp_string &appType)
{
    return nullptr;
}

std::shared_ptr<thriftservice::IThriftClient> StubGetPluginClient(void* obj)
{
    return std::make_shared<ThriftClientStub>();
}

std::shared_ptr<thriftservice::IThriftClient> StubGetPluginClient_Null(void* obj)
{
    return nullptr;
}

mp_int32 Ret_Succ(){
    return MP_SUCCESS;
}

mp_int32 Ret_Fail(){
    return MP_FAILED;
}

bool Ret_False(){
    return false;
}

bool Ret_True(){
    return true;
}

class StubProtectServiceClient : virtual public AppProtect::ProtectServiceClient {
public:
    StubProtectServiceClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}
	
	void DeliverTaskStatus(ActionResult& _return, const std::string& status, const std::string& jobId, const std::string& script) override
    {
        _return.code = MP_SUCCESS;
    }
};

static std::shared_ptr<StubProtectServiceClient> g_ProtectServiceClient;
static std::shared_ptr<AppProtect::ProtectServiceClient> GetProtectServiceClientStub(
    mp_void* pThis, const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient)
{
    if (g_ProtectServiceClient.get() == nullptr) {
        g_ProtectServiceClient = std::make_shared<StubProtectServiceClient>(nullptr);
    }
    return g_ProtectServiceClient;
}

/*
*用例名称：获取AppProtectService
*前置条件：AppProtectService Init初始化成功
*check点：返回句柄不为空
*/
TEST_F(AppProtectServiceTest, GetInstance)
{
    auto AppProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(AppProtectServiceIns, nullptr);

    AppProtectService::g_instance = nullptr;
    stub.set(ADDR(AppProtectService, Init), StubInitFailed);
    AppProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_EQ(AppProtectServiceIns, nullptr);
}

TEST_F(AppProtectServiceTest, WakeUpJob)
{
    stub.set(&CIP::CheckIsDoradoEnvironment, StubTestCheckIsDoradoEnvironmentSuccess);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetDoraDoLanNet), StubGetDoraDoLanNet);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, InitializeTimer), StubSUCCESS);
    stub.set(ADDR(CHost, GetContainerIPList), StubSUCCESS);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, WakeUpJob), StubSUCCESS);

    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    CRequestMsg req;
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/notify";
    req.m_url.m_queryParam["subTaskId"] = "1a645f40-41b5-4a71-b8fc-1ba1f13a1111";
    req.m_msgBody.m_msgJsonData["appType"] = "appType";
    CResponseMsg rsp;
    mp_int32 ret = appProtectServiceIns->WakeUpJob(req, rsp);
    EXPECT_EQ(MP_SUCCESS, ret);
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/notify";
    ret = appProtectServiceIns->WakeUpJob(req, rsp);
    EXPECT_EQ(MP_SUCCESS, ret);

    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetInstance), StubGetInstanceNull);
    ret = appProtectServiceIns->WakeUpJob(req, rsp);
    EXPECT_EQ(MP_FAILED, ret);
    stub.set(ADDR(AppProtectService, GetTaskIdFromUrl), StubGetTaskIdFromUrlEmpty);
    ret = appProtectServiceIns->WakeUpJob(req, rsp);
    EXPECT_EQ(MP_FAILED, ret);
}

/*
*用例名称：调用中止任务接口成功
*前置条件：URL格式正确
*check点：URL格式正确，可以中止
*/
TEST_F(AppProtectServiceTest, AbortSuccess)
{
    stub.set((mp_int32(AppProtect::AppProtectJobHandler::*)(const std::string&, const std::string&))
        ADDR(AppProtect::AppProtectJobHandler, AbortJob), StubAbortJob);
    stub.set(&CIP::CheckIsDoradoEnvironment, StubTestCheckIsDoradoEnvironmentSuccess);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetDoraDoLanNet), StubGetDoraDoLanNet);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, InitializeTimer), StubSUCCESS);
    stub.set(ADDR(CHost, GetContainerIPList), StubSUCCESS);

    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    CRequestMsg req;
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/abort";
    req.m_url.m_queryParam["subTaskId"] = "1a645f40-41b5-4a71-b8fc-1ba1f13a1111";
    CResponseMsg rsp;
    mp_int32 ret = appProtectServiceIns->AbortJob(req, rsp);
    EXPECT_EQ(MP_SUCCESS, ret);
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/abort";
    ret = appProtectServiceIns->AbortJob(req, rsp);
    EXPECT_EQ(MP_SUCCESS, ret);
}
/*
*用例名称：调用中止任务接口失败，因为无法从URL中获取到有效信息
*前置条件：URL格式错误
*check点：URL格式错误，无法发起中止
*/
TEST_F(AppProtectServiceTest, AbortFailForIleagalUrl)
{
    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    CRequestMsg req;
    req.m_url.m_procURL = "/v1/agent/tasks/b8fc-1ba1f13a6403/abort";
    CResponseMsg rsp;
    mp_int32 ret = appProtectServiceIns->AbortJob(req, rsp);
    EXPECT_EQ(MP_FAILED, ret);
}

TEST_F(AppProtectServiceTest, AbortJobFailed)
{
    stub.set((mp_int32(AppProtect::AppProtectJobHandler::*)(const std::string&, const std::string&))
        ADDR(AppProtect::AppProtectJobHandler, AbortJob), StubAbortJob);
    stub.set(&CIP::CheckIsDoradoEnvironment, StubTestCheckIsDoradoEnvironmentSuccess);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetDoraDoLanNet), StubGetDoraDoLanNet);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, InitializeTimer), StubSUCCESS);
    stub.set(ADDR(CHost, GetContainerIPList), StubSUCCESS);

    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    CRequestMsg req;
    req.m_url.m_queryParam["subTaskId"] = "1a645f40-41b5-4a71-b8fc-1ba1f13a1111";
    CResponseMsg rsp;
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/abort";

    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetInstance), StubGetInstanceNull);
    mp_int32 ret = appProtectServiceIns->AbortJob(req, rsp);
    EXPECT_EQ(MP_FAILED, ret);
    stub.set(ADDR(AppProtectService, GetTaskIdFromUrl), StubGetTaskIdFromUrlEmpty);
    ret = appProtectServiceIns->AbortJob(req, rsp);
    EXPECT_EQ(MP_FAILED, ret);
}

/*
*用例名称：调用通知任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, DeliverJobStatusSuccess)
{
    stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    stub.set((&AppProtectService::GetProtectServiceClient), GetProtectServiceClientStub);

    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    mp_string appType = "test";
    Json::Value JsonReq;
    JsonReq["status"] = "SUCCESS";
    JsonReq["taskId"] = "123";
    JsonReq["script"] = "hana";
    CRequestMsg req;
    req.SetJsonData(JsonReq);
    CResponseMsg rsp;
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->DeliverJobStatus(appType, req, rsp));
    EXPECT_EQ("0", rsp.GetJsonValueRef()["errorCode"].asString());
    stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient_Null);
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->DeliverJobStatus(appType, req, rsp));
    stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest_Null);
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->DeliverJobStatus(appType, req, rsp));
    stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
}

TEST_F(AppProtectServiceTest, SanclientJobForUbcT)
{
    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    Json::Value jvReq;
    CRequestMsg req;
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->SanclientJobForUbc(jvReq, req));
}

TEST_F(AppProtectServiceTest, SanclientMountEXT)
{
    Json::Value repositories;
    Json::Value remotePathJson;
    AppProtect::MountNasParam param;
    AppProtect::FilesystemInfo fileinfo;
    StorageRepository stRep;
    remotePathJson["path"] = "path";
    repositories["type"] = 1;
    remotePathJson["type"] = 1;
    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    Json::Value jvReq;
    CRequestMsg req;
    appProtectServiceIns->SanclientMountEX(repositories, remotePathJson, param, fileinfo, stRep);
}

mp_int32 StubMountNasFileSystemSuccess(void *obj, AppProtect::MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, 
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const AppProtect::MountPermission &permit)
{
    successMountPath.push_back("192.168.1.1");
    return MP_SUCCESS;
}

mp_int32 StubMountDataturboFileSystemSuccess(void *obj, AppProtect::MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, std::vector<mp_string> &dtbMountPath,
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const AppProtect::MountPermission &permit)
{
    successMountPath.push_back("192.168.1.1");
    return MP_SUCCESS;
}

TEST_F(AppProtectServiceTest, MountTypeT)
{
    stub.set(&AppProtect::PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystemSuccess);
    stub.set(&AppProtect::PrepareFileSystem::MountDataturboFileSystem, StubMountDataturboFileSystemSuccess);
    mp_string lanFreeSwitch;
    AppProtect::MountNasParam param;
    param.authPwd = "password";
    param.jobID = "1q1qa1";
    AppProtect::FilesystemInfo fileinfo;
    fileinfo.mountPoints.push_back("kldjsojdi");
    fileinfo.mountPoints.push_back("jdhjfkldjkl");
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    auto appProtectServiceIns = AppProtectService::GetInstance();
    EXPECT_NE(appProtectServiceIns, nullptr);
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->MountType(lanFreeSwitch, param, fileinfo, filesysteminfo));
    lanFreeSwitch = "true";
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->MountType(lanFreeSwitch, param, fileinfo, filesysteminfo));
}

/*
*用例名称：调用通知env任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, EnvCheckSuccess)
{
    stub.set(ADDR(CRootCaller, Exec), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->EnvCheck(taskid));

}

/*
*用例名称：调用通知env任务状态接口成功
*前置条件
*check点：接口调用失败
*/
TEST_F(AppProtectServiceTest, EnvCheckFailed)
{
    stub.set(ADDR(CRootCaller, Exec), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->EnvCheck(taskid));
}

/*
*用例名称：ISCSI协议调用创建lun任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, CreateLunISCSISuccess)
{
    // stub.set((&AppProtectService::CreateLunForIqns), StubSUCCESS);
    stub.set(ADDR(CRootCaller, Exec), StubSUCCESS);
    stub.set(&CIP::GetListenIPAndPort, StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    appProtectServiceIns->m_lunidList = {1, 2, 3};
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cq";
    mp_string agentIqns = "144444444";
    mp_string iqns = "11111";
    AppProtect::FilesystemInfo filesysteminfo1;
    filesysteminfo1.FilesystemMountPath = "/clone_dsadqdqsdasdqdqdqdqa/full_dsadvqwvqdqbdqwdqhdq";
    filesysteminfo1.FilesystemName = "qqqqq";
    filesysteminfo1.Filesystemtype = 1;
    AppProtect::FilesystemInfo filesysteminfo2;
    filesysteminfo2.FilesystemName = "wwwww";
    std::vector<AppProtect::FilesystemInfo> filesysteminfo3 = {filesysteminfo1, filesysteminfo2};
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo3, taskid));
}
 
/*
*用例名称：ISCSI协议调用创建lun任务状态接口失败
*前置条件
*check点：接口调用失败
*/
TEST_F(AppProtectServiceTest, CreateLunISCSIFailedV1)
{
    stub.set((&AppProtectService::CreateLunForIqns), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872ca";
    mp_string agentIqns = "144444444";
    mp_string iqns = "11111";
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo, taskid));
 
    AppProtect::FilesystemInfo filesysteminfo1;
    filesysteminfo1.FilesystemName = "qqqqq";
    AppProtect::FilesystemInfo filesysteminfo2;
    filesysteminfo2.FilesystemName = "wwwww";
    std::vector<AppProtect::FilesystemInfo> filesysteminfo3 = {filesysteminfo1, filesysteminfo2};
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo3, taskid));
 
    stub.set(&CIP::GetListenIPAndPort, StubSUCCESS);
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo3, taskid));
}
 
TEST_F(AppProtectServiceTest, CreateLunISCSIFailedV2)
{
    stub.set(ADDR(CRootCaller, Exec), StubInitFailed);
    stub.set(&CIP::GetListenIPAndPort, StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872ca";
    mp_string agentIqns = "144444444";
    mp_string iqns = "11111";
    AppProtect::FilesystemInfo filesysteminfo1;
    filesysteminfo1.FilesystemName = "qqqqq";
    filesysteminfo1.Filesystemtype = 1;
    filesysteminfo1.BackupCopiesID = { "aaaa", "bbbb" };
    std::vector<AppProtect::FilesystemInfo> filesysteminfo3 = {filesysteminfo1};
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo3, taskid));
 
    stub.set(&CIP::GetListenIPAndPort, StubSUCCESS);
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLunISCSI(agentIqns, iqns, filesysteminfo3, taskid));
}

/*
*用例名称：调用创建lun任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, CreateLunFailed)
{
    stub.set((&AppProtectService::CreateLunForWwpns), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    std::vector<mp_string> agentwwpns = {"144444444","155555555"};
    std::vector<mp_string> wwpns = {"11111", "22222"};
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLun(agentwwpns, wwpns,  filesysteminfo, taskid));

    AppProtect::FilesystemInfo filesysteminfo1;
    filesysteminfo1.FilesystemName = "qqqqq";
    AppProtect::FilesystemInfo filesysteminfo2;
    filesysteminfo2.FilesystemName = "wwwww";
    std::vector<AppProtect::FilesystemInfo> filesysteminfo3 = {filesysteminfo1, filesysteminfo2};
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLun(agentwwpns, wwpns,  filesysteminfo3, taskid));
}

/*
*用例名称：调用通知env任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, CreateLunSuccess)
{
    stub.set((&AppProtectService::CreateLunForWwpns), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    std::vector<mp_string> agentwwpns = {"144444444","155555555"};
    std::vector<mp_string> wwpns = {"11111", "22222"};
    AppProtect::FilesystemInfo filesysteminfo1;
    filesysteminfo1.FilesystemName = "qqqqq";
    AppProtect::FilesystemInfo filesysteminfo2;
    filesysteminfo2.FilesystemName = "wwwww";
    std::vector<AppProtect::FilesystemInfo> filesysteminfo = {filesysteminfo1, filesysteminfo2};
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->CreateLun(agentwwpns, wwpns,  filesysteminfo, taskid));
}

/*
*用例名称：调用通知cleanenv任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, CleanEnvFailed)
{
    stub.set(ADDR(CRootCaller, Exec), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    AppProtect::LunInfo luinfo;
    luinfo.Wwpn = "1234444";
    luinfo.LunName = "123";
    luinfo.LunId = "1";
    luinfo.AgentWwpn = "222222";
    luinfo.Path = "/test/mnt";
    luinfo.FileioName = "test";
    luinfo.FilesystemSize = 32;
    luinfo.mountPoints = {"/test/mnt", "/test/mnt1"};
    appProtectServiceIns->m_lunInfos = {luinfo};
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CleanEnv(taskid));
}

/*
*用例名称：调用通知cleanenv任务状态接口成功
*前置条件
*check点：接口调用失败
*/
TEST_F(AppProtectServiceTest, CleanEnvSuccess)
{
    stub.set(ADDR(CRootCaller, Exec), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->CleanEnv(taskid));
}

/*
*用例名称：调用通知cleanenv任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, SanclientMountSuccess)
{
    stub.set((&AppProtectService::MountType), StubSUCCESS);
    stub.set(ADDR(CJsonUtils, GetJsonArrayJson), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    mp_string strValue = "{ \"repositories\" : [{ \"type\" : 1, \"protocol\" : \
        1, \"remoteHost\" : [{ \"ip\" : \"192.168.145.161\" } ], \"remotePath\" : \"/Share1\" } ] }";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
     mp_string lanFreeSwitch = "false";
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->SanclientMount(jsValue, taskid, filesysteminfo, lanFreeSwitch));
}

TEST_F(AppProtectServiceTest, CreateLunFailedV1)
{
    stub.set(ADDR(CRootCaller, Exec), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872ch";
    std::vector<mp_string> agentwwpns = {"144444444"};
    std::vector<mp_string> wwpns = {"11111"};
    AppProtect::FilesystemInfo fileinfo;
    fileinfo.FilesystemMountPath = "/mnt/test";
    fileinfo.FilesystemName = "test";
    fileinfo.Filesystemtype = 1;
    fileinfo.FilesystemSize = 32;
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    filesysteminfo.push_back(fileinfo);
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->CreateLun(agentwwpns, wwpns,  filesysteminfo, taskid));
}

TEST_F(AppProtectServiceTest, CreateLunSuccessV2)
{
    stub.set(ADDR(CRootCaller, Exec), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    std::vector<mp_string> agentwwpns = {"155555555"};
    std::vector<mp_string> wwpns = {"33333"};
    AppProtect::FilesystemInfo fileinfo;
    fileinfo.FilesystemMountPath = "/clone_dsadqdqsdasdqdqdqdqa/full_dsadvqwvqdqbdqwdqhdq";
    fileinfo.FilesystemName = "test";
    fileinfo.Filesystemtype = 1;
    fileinfo.FilesystemSize = 32;
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    filesysteminfo.push_back(fileinfo);
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->CreateLun(agentwwpns, wwpns, filesysteminfo, taskid));
}

TEST_F(AppProtectServiceTest, SanclientMountSuccessV2)
{
    stub.set((&AppProtectService::MountType), StubSUCCESS);
    stub.set(ADDR(CRootCaller, Exec), StubInitFailed);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    AppProtect::FilesystemInfo fileinfo;
    fileinfo.FilesystemMountPath = "/clone_dsadqdqsdasdqdqdqdqa/full_dsadvqwvqdqbdqwdqhdq";
    fileinfo.FilesystemName = "test";
    fileinfo.Filesystemtype = 1;
    fileinfo.FilesystemSize = 32;
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
    filesysteminfo.push_back(fileinfo);
    Json::Value jvReq;
    mp_string lanFreeSwitch = "true";
    mp_string jvReqstring = "{\"agentWwpns\":[\"c05076032e730022\"],\"dmeIpList\":[\"129.115.132.73\",\"129.115.132.72\"],\"repositories\":[{\"auth\":{\"authKey\":\"data_turbo_account\",\"authPwd\":\"10N@C5tv|&-3+)z3\",\"authType\":2,\"extendInfo\":null},\"endpoint\":{\"agentOS\":null,\"id\":null,\"ip\":\"8.44.135.13,8.44.135.14\",\"iqns\":null,\"port\":8088,\"sanClients\":null,\"status\":null,\"supportProtocol\":null,\"wwpns\":null},\"extendAuth\":{\"authKey\":\"dataprotect_admin\",\"authPwd\":\"kR%76!BP4!&!\",\"authType\":2,\"extendInfo\":null},\"extendInfo\":{\"capacity\":35184372088832,\"copy_format\":1,\"esn\":\"2102354PBB10N5100002\"},\"id\":null,\"isLocal\":true,\"path\":null,\"protocol\":1,\"remoteHost\":[{\"agentOS\":null,\"id\":null,\"ip\":\"129.115.132.60\",\"iqns\":null,\"port\":null,\"sanClients\":null,\"status\":null,\"supportProtocol\":3,\"wwpns\":null},{\"agentOS\":null,\"id\":null,\"ip\":\"129.115.132.64\",\"iqns\":null,\"port\":null,\"sanClients\":null,\"status\":null,\"supportProtocol\":3,\"wwpns\":null}],\"remotePath\":[{\"id\":\"40\",\"parentId\":null,\"path\":\"/Database_MetaDataRepository/669b8105-625f-53d4-9da9-eeb1474f4a15\",\"remoteHost\":[{\"agentOS\":null,\"id\":null,\"ip\":\"129.115.132.64\",\"iqns\":null,\"port\":null,\"sanClients\":null,\"status\":null,\"supportProtocol\":3,\"wwpns\":null}],\"shareName\":\"669b8105-625f-53d4-9da9-eeb1474f4a15_meta\",\"type\":1}],\"role\":0,\"type\":0},{\"auth\":{\"authKey\":\"data_turbo_account\",\"authPwd\":\"10N@C5tv|&-3+)z3\",\"authType\":2,\"extendInfo\":null},\"endpoint\":{\"agentOS\":null,\"id\":null,\"ip\":\"8.44.135.13,8.44.135.14\",\"iqns\":null,\"port\":8088,\"sanClients\":null,\"status\":null,\"supportProtocol\":null,\"wwpns\":null},\"extendAuth\":{\"authKey\":\"dataprotect_admin\",\"authPwd\":\"kR%76!BP4!&!\",\"authType\":2,\"extendInfo\":null},\"extendInfo\":{\"capacity\":35184372088832,\"esn\":\"2102354PBB10N5100002\"},\"id\":null,\"isLocal\":true,\"path\":null,\"protocol\":1,\"remoteHost\":[{\"agentOS\":null,\"id\":null,\"ip\":\"129.115.132.60\",\"iqns\":null,\"port\":null,\"sanClients\":null,\"status\":null,\"supportProtocol\":3,\"wwpns\":null},{\"agentOS\":null,\"id\":null,\"ip\":\"129.115.132.64\",\"iqns\":null,\"port\":null,\"sanClients\":null,\"status\":null,\"supportProtocol\":3,\"wwpns\":null}],\"requestId\":\"447f99aa-5c36-4240-9289-7eb831d1c941\",\"sanclient\":{\"fcPorts\":null,\"id\":\"a8b50c55-8cfc-4039-9742-a2912da9b508_sanclient\",\"ip\":\"129.115.190.3\",\"iqns\":null,\"openLanFreeSwitch\":false,\"port\":59577,\"sanClientWwpns\":[\"21000024ff0dc3be\"],\"status\":\"notifying\",\"wwpns\":null},\"taskID\":\"447f99aa-5c36-4240-9289-7eb831d1c941\"}";
    CJsonUtils::ConvertStringtoJson(jvReqstring, jvReq);
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->SanclientMount(jvReq, taskid, filesysteminfo, lanFreeSwitch));
}

/*
*用例名称：调用通知UBC任务状态接口成功
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, SanclientJobForUbcFailed)
{
    Json::Value jvReq;
    CRequestMsg req;
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    EXPECT_EQ(MP_FAILED, appProtectServiceIns->SanclientJobForUbc(jvReq, req));
}


TEST_F(AppProtectServiceTest, SanclientJobForUbcSuccess)
{
    auto appProtectServiceIns = AppProtectService::GetInstance();
    Json::Value jvReq;
    CRequestMsg req;
    req.m_url.m_procURL = "/agent/v1/tasks/e86b01df-85be-46fb-bb62-5249e609859c/abort";
    req.m_url.m_queryParam["subTaskId"] = "1a645f40-41b5-4a71-b8fc-1ba1f13a1111";
     EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->SanclientJobForUbc(jvReq, req));
}

/*
*用例名称：挂载sanclientmount失败
*前置条件
*check点：接口调用成功
*/
TEST_F(AppProtectServiceTest, SanclientMountSuccess1)
{
    stub.set(&AppProtect::PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(ADDR(CJsonUtils, GetJsonArrayJson), StubSUCCESS);
    auto appProtectServiceIns = AppProtectService::GetInstance();
    mp_string taskid = "f7c624b3-18e3-4f58-8980-f9e5a27872cd";
    mp_string strValue = "{ \"repositories\" : [{ \"type\" : 1, \"protocol\" : \
        1, \"remoteHost\" : [{ \"ip\" : \"192.168.145.161\" } ], \"remotePath\" : \"/Share1\" } ] ,""}";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    std::vector<AppProtect::FilesystemInfo> filesysteminfo;
     mp_string lanFreeSwitch = "false";
    EXPECT_EQ(MP_SUCCESS, appProtectServiceIns->SanclientMount(jsValue, taskid, filesysteminfo, lanFreeSwitch));
}
