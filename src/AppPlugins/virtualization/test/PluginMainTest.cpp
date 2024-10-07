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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <iostream>
#include <list>
#include <cstdio>
#include "PluginMain.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "protect_engines/ProtectEngineMock.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include <curl_http/HttpClientInterface.h>
#include "common/model/ResponseModel.h"
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "protect_engines/hcs/resource_discovery/HcsResourceAccess.h"
#include "common/httpclient/HttpClient.h"
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "job/JobCommonInfo.h"
#include "job_controller/jobs/backup/BackupJob.h"
#include "volume_handlers/oceanstor/OceanStorVolumeHandler.h"
#include "volume_handlers/fusionstorage/FusionStorageVolumeHandler.h"
#include "protect_engines/hcs/HCSProtectEngine.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;
using ::testing::A;

using namespace VirtPlugin;
using namespace AppProtect;
namespace {
const int32_t MAX_TEST_SIZE = 100;
}
namespace HDT_TEST {

const std::string g_SnapshotInfo = "{\
    \"extendInfo\": \"\",\
    \"moRef\": \"\",\
    \"preMoRef\": \"\",\
    \"vmName\": \"\",\
    \"vmUuid\": \"\",\
    \"volSnapList\": null\
}";

const std::string ENV_APPTYPE_HCS = "HCSContainer";

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_DiscoverApplications_exists(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    std::vector<Application> returnValue;
    Application app {};
    returnValue.push_back(app);
    EXPECT_CALL(*protectEngine, DiscoverApplications(_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(returnValue)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_CheckApplication_OK(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    ActionResult actionResult {};
    actionResult.__set_code(0);
    EXPECT_CALL(*protectEngine, CheckApplication(_,_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(actionResult)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_ListApplicationResource_exists(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    std::vector<ApplicationResource> returnValue;
    ApplicationResource applicationResource {};
    returnValue.push_back(applicationResource);
    EXPECT_CALL(*protectEngine, ListApplicationResource(_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(returnValue)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_ListApplicationResourceV2_exists(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    ResourceResultByPage page {};
    ApplicationResource applicationResource {};
    page.items.push_back(applicationResource);
    EXPECT_CALL(*protectEngine, ListApplicationResourceV2(_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(page)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_DiscoverHostCluster_exists(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    ApplicationEnvironment returnValue {};
    ApplicationEnvironment env {};
    returnValue.nodes.push_back(env);
    EXPECT_CALL(*protectEngine, DiscoverHostCluster(_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(returnValue)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_DiscoverAppCluster_exists(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    ApplicationEnvironment returnValue {};
    ApplicationEnvironment env {};
    returnValue.nodes.push_back(env);
    EXPECT_CALL(*protectEngine, DiscoverAppCluster(_,_,_)).WillRepeatedly(DoAll(SetArgReferee<0>(returnValue)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_CheckBackupJobType_Failed(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    EXPECT_CALL(*protectEngine, CheckBackupJobType(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(false),
        Return(SUCCESS)));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_CheckBackupJobType_OK(const std::string appType)
{
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>();
    EXPECT_CALL(*protectEngine, CheckBackupJobType(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(true),
        Return(SUCCESS)));
    return protectEngine;
}

static int32_t FileSystemReadSnapshotSuccess_Invoke(std::string &buf, size_t size)
{
    buf = g_SnapshotInfo;
    return g_SnapshotInfo.length();
}

static int32_t FileSystemWriteSuccess_Invoke(const std::string &content)
{
    return content.length();
}

int32_t Stub_CheckHcsSuccess(ApplicationEnvironment &returnEnv)
{
    return SUCCESS;
}

int32_t Stub_CheckStorageEnvConnSuccess(const ApplicationEnvironment &appEnv)
{
    return SUCCESS;
}

const std::string t_serverDetail =
    "{\"server\":{\"tenant_id\":\"e38d227edcce4631be20bfa5aad7130b\",\"addresses\":{\"subnet-8f61\":[{\"OS-EXT-IPS-MAC:"
    "mac_addr\":\"fa:16:3e:0b:7a:38\",\"OS-EXT-IPS:type\":\"fixed\",\"addr\":\"192.168.0.216\",\"version\":4}]},"
    "\"metadata\":{\"productId\":\"5b4ecaa32947446b824df4a6c60c8a04\",\"__instance_vwatchdog\":\"false\",\"_ha_policy_"
    "type\":\"remote_rebuild\",\"server_expiry\":\"0\",\"cascaded.instance_extrainfo\":\"max_cpu:254,current_mem:8192,"
    "org_mem:8192,iohang_timeout:720,pcibridge:2,system_serial_number:10a4e361-c981-46f2-b9ba-d7ff9c601693,max_mem:"
    "4194304,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:True,current_cpu:4,uefi_mode_sysinfo_fields:"
    "version_serial_uuid_family_asset,num_of_mem_plug:57\"},\"OS-EXT-STS:task_state\":null,\"OS-DCF:diskConfig\":"
    "\"MANUAL\",\"OS-EXT-AZ:availability_zone\":\"az0.dc0\",\"links\":[{\"rel\":\"self\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"},{\"rel\":\"bookmark\",\"href\":\"https://ecs.sc-cd-1.demo.com/"
    "e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"}],\"OS-EXT-STS:power_state\":4,\"id\":\"10a4e361-c981-46f2-b9ba-"
    "d7ff9c601693\",\"os-extended-volumes:volumes_attached\":[{\"id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\"}],\"OS-"
    "EXT-SRV-ATTR:host\":\"EA918B93-2561-E611-9B2A-049FCAD22DFC\",\"image\":{\"links\":[{\"rel\":\"bookmark\",\"href\":"
    "\"https://ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/images/"
    "a0a24ff5-1899-4c38-843a-659fd9d3ac15\"}],\"id\":\"a0a24ff5-1899-4c38-843a-659fd9d3ac15\"},\"OS-SRV-USG:terminated_"
    "at\":null,\"accessIPv4\":\"\",\"accessIPv6\":\"\",\"created\":\"2022-05-11T03:00:26Z\",\"hostId\":"
    "\"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\"OS-EXT-SRV-ATTR:hypervisor_hostname\":\"EA918B93-"
    "2561-E611-9B2A-049FCAD22DFC\",\"key_name\":null,\"flavor\":{\"links\":[{\"rel\":\"bookmark\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/flavors/"
    "ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"}],\"id\":\"ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"},\"security_groups\":[{"
    "\"name\":\"default\"}],\"config_drive\":\"\",\"OS-EXT-STS:vm_state\":\"stopped\",\"OS-EXT-SRV-ATTR:instance_"
    "name\":\"instance-00000030\",\"user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"name\":\"ecs-4d45-0001\",\"OS-SRV-"
    "USG:launched_at\":\"2022-05-11T03:00:36.000000\",\"updated\":\"2022-06-28T09:54:06Z\",\"status\":\"SHUTOFF\"}}";

int32_t Stub_GetServerDetailSuccess(
    void *obj, const Module::HttpRequest &request, std::shared_ptr<VirtPlugin::ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(t_serverDetail);
    response->SetStatusCode(static_cast<uint32_t>(HcsPlugin::HcsExternalStatusCode::OK));
    return SUCCESS;
}

int32_t Stub_GetServerDetailFail(
    void *obj, const Module::HttpRequest &request, std::shared_ptr<VirtPlugin::ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(static_cast<uint32_t>(HcsPlugin::HcsExternalStatusCode::NOT_FOUND));
    return SUCCESS;
}

void SetEnvObj(AppProtect::ApplicationEnvironment &env)
{
    env.__set_id("136");
    env.__set_name("HcsPlanet");
    env.__set_type("Virtual");
    env.__set_endpoint("demo.com");
    env.auth.__set_authkey("bss_admin");
    env.auth.__set_authPwd("xxxxxxxx");
    env.auth.__set_extendInfo(
        "{\"vdcInfo\":\"{\\\"name\\\":\\\"huangrong\\\", \\\"passwd\\\":\\\"Huawei\\\","
        "\\\"domainName\\\":\\\"sc-sh\\\"}\",\"certification\":\"\",\"storages\":\"\"}");
    env.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");
}

bool Stub_TokenSuccess(void *obj, VirtPlugin::ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = "mocktoken";
    endPoint = "mockendPoint";
    return true;
}

bool Stub_FusionStorage_CheckHealthStatus_Success(std::string &errDesc)
{
    return true;
}

int32_t Stub_FusionStorage_TestDeviceConnection_Success(const std::string &authExtendInfo, int32_t &erroCode)
{
    return SUCCESS;
}

bool Stub_FusionStorage_CheckHealthStatus_Failed(std::string &errDesc)
{
    return false;
}

int32_t Stub_FusionStorageRestApiOperator_GetCurrentSession_Failed(bool &isNetworkOk)
{
    return FAILED;
}

int32_t Stub_FusionStorage_TestDeviceConnection_Failed(const std::string &authExtendInfo, int32_t &erroCode)
{
    return FAILED;
}

int32_t Stub_OceanStor_TestDeviceConnection_Success()
{
    return SUCCESS;
}

int32_t Stub_OceanStor_TestDeviceConnection_Failed()
{
    return FAILED;
}

int32_t Stub_HcsGetStorageType(const ApplicationEnvironment &appEnv, std::string &storageType){
    storageType = "0";
    return FAILED;
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(g_SnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

class PluginMainTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    /**
    * @brief 组装Agent下发的副本数据
    *
    * @param backupJobParam
    * @return def
    */
    void FormCopyData(AppProtect::BackupJob &backupJobParam)
    {
        // 数据仓
        StorageRepository dataRepo;
        dataRepo.__set_repositoryType(RepositoryDataType::DATA_REPOSITORY);
        dataRepo.path.push_back("/datapath1");
        backupJobParam.repositories.push_back(dataRepo);
        // 元数据仓
        StorageRepository metaRepo;
        metaRepo.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
        metaRepo.path.push_back("/metapath1");
        backupJobParam.repositories.push_back(metaRepo);
        // 缓存仓
        StorageRepository cacheRepo;
        cacheRepo.__set_repositoryType(RepositoryDataType::CACHE_REPOSITORY);
        cacheRepo.path.push_back("/cachepath1");
        backupJobParam.repositories.push_back(cacheRepo);
    }

    void FormJobInfo(const std::string &storageType)
    {
        m_backupSubJobInfo.m_volInfo.m_moRef = "vm_moref";
        m_backupSubJobInfo.m_volInfo.m_volSizeInBytes = MAX_TEST_SIZE * DIRTY_RANGE_BLOCK_SIZE;
        m_backupSubJobInfo.m_volInfo.m_datastore.m_moRef = "ds_moref";
        m_backupSubJobInfo.m_volInfo.m_datastore.m_ip = "ip";
        m_backupSubJobInfo.m_volInfo.m_datastore.m_poolId = "poolId";
        m_backupSubJobInfo.m_volInfo.m_datastore.m_type = storageType;
        m_backupSubJob.jobId = "main_job_id";
        m_backupSubJob.subJobId = "sub_job_id";
        std::string backupSubJobInfoStr;
        Module::JsonHelper::StructToJsonString(m_backupSubJobInfo, backupSubJobInfoStr);
        m_backupSubJob.jobInfo = backupSubJobInfoStr;
        SetEnvObj(m_backupJobInfo.protectEnv);
        std::string authExtendInfo = "{\"vdcInfo\":\"{\\\"name\\\":\\\"huangrong\\\", \\\"passwd\\\":\\\"Huawei\\\","
            "\\\"domainName\\\":\\\"sc-sh\\\"}\",\"certification\":\"\","
            "\"storages\":\"[{\\\"sn\\\": \\\"ds_moref\\\",\\\"username\\\": \\\"admin\\\","
            "\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"1.1.1.1\\\",\\\"port\\\": 8088}]\"}";
        m_backupJobInfo.protectEnv.auth.__set_extendInfo(authExtendInfo);
        m_backupJobInfo.protectEnv.subType = ENV_APPTYPE_HCS;

        VolInfo volInfo;
        volInfo.m_moRef = "moref";
        volInfo.m_name = "name";
        volInfo.m_type = "m_type";
        volInfo.m_datastore.m_type = storageType;
        volInfo.m_datastore.m_moRef = "ds_moref";
        volInfo.m_datastore.m_name = "ds_name";
        volInfo.m_datastore.m_ip = "ip";
        volInfo.m_datastore.m_poolId = "poolId";
        volInfo.m_volSizeInBytes = MAX_TEST_SIZE * DIRTY_RANGE_BLOCK_SIZE;
        std::string volStr;
        Module::JsonHelper::StructToJsonString(volInfo, volStr);
        m_restoreSubJobExtendInfo.m_targetVolumeInfo = volStr;
        m_restoreSubJob.jobId = "main_job_id";
        m_restoreSubJob.subJobId = "sub_job_id";
        std::string restoreSubJobInfoStr;
        Module::JsonHelper::StructToJsonString(m_restoreSubJobExtendInfo, restoreSubJobInfoStr);
        m_restoreSubJob.jobInfo = restoreSubJobInfoStr;
        SetEnvObj(m_restoreJobInfo.targetEnv);
        m_restoreJobInfo.targetEnv.auth.__set_extendInfo(authExtendInfo);
        m_restoreJobInfo.targetEnv.subType = ENV_APPTYPE_HCS;
    }
public:
    AppProtect::BackupJob m_backupJobInfo;
    AppProtect::RestoreJob m_restoreJobInfo;
    AppProtect::SubJob m_backupSubJob;
    AppProtect::SubJob m_restoreSubJob;
    BackupSubJobInfo m_backupSubJobInfo;
    SubJobExtendInfo m_restoreSubJobExtendInfo;
};

/*
 * 测试用例： 初始化日志
 * 前置条件： 传入日志路径为空
 * CHECK点： 初始化日志成功
 */
TEST_F(PluginMainTest, AppInitNoParaSuccess)
{
    std::string logPath = "";
    int32_t retValue = AppInit(logPath);
    EXPECT_EQ(retValue, SUCCESS);
}

/*
 * 测试用例： 初始化日志
 * 前置条件： 传入日志路径不为空
 * CHECK点： 初始化日志成功
 */
TEST_F(PluginMainTest, AppInitHaveParaSuccess)
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string logPath = agentHomedir + "/OceanProtect/ProtectClient/ProtectClient-E/slog/VirtualPlugin/log";
    int32_t retValue = AppInit(logPath);
    EXPECT_EQ(retValue, SUCCESS);
}

/*
 * 测试用例： 发现资源
 * 前置条件： 应用实例创建成功
 * CHECK点： 能正确返回查询到的资源个数
 */
TEST_F(PluginMainTest, DiscoverApplicationsExists)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_DiscoverApplications_exists);
    std::vector<Application> returnValue;
    std::string appType = "apptype";
    DiscoverApplications(returnValue, appType);
    int nApp = returnValue.size();
    EXPECT_EQ(nApp, 1);
}

/*
 * 测试用例： 检查应用存在
 * 前置条件： 应用实例创建成功
 * CHECK点： 查询到应用是returnValue.code为0
 */
TEST_F(PluginMainTest, CheckApplicationOk)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_CheckApplication_OK);
    ActionResult returnValue {};
    ApplicationEnvironment appEnv {};
    Application application {};
    CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(returnValue.code, 0);
}

/*
 * 测试用例： 检查应用参数校验失败
 * 前置条件： 应用实例创建成功
 * CHECK点： Application参数校验
 */
TEST_F(PluginMainTest, CheckApplicationParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_CheckApplication_OK);
    ActionResult returnValue {};
    ApplicationEnvironment appEnv {};
    Application application {};
    application.parentName = "parent*name%";
    CheckApplication(returnValue, appEnv, application);
    EXPECT_EQ(returnValue.code, 0x4003291A);
}

/*
 * 测试用例： 批量查询资源
 * 前置条件： 应用实例创建成功
 * CHECK点： 能正确返回查询到的资源个数
 */
TEST_F(PluginMainTest, ListApplicationResourceExists)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_ListApplicationResource_exists);
    std::vector<ApplicationResource> returnValue {};
    ApplicationEnvironment appEnv {};
    Application application {};
    ApplicationResource parentResource {};
    ListApplicationResource(returnValue, appEnv, application, parentResource);
    EXPECT_EQ(returnValue.size(), 1);
}

/*
 * 测试用例： 批量查询资源参数校验失败
 * 前置条件： 应用实例创建成功
 * CHECK点： ApplicationResource参数校验
 */
TEST_F(PluginMainTest, ListApplicationResourceParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_ListApplicationResource_exists);
    std::vector<ApplicationResource> returnValue {};
    ApplicationEnvironment appEnv {};
    Application application {};
    ApplicationResource parentResource {};
    parentResource.port = 65536;
    try {
        ListApplicationResource(returnValue, appEnv, application, parentResource);
    } catch (...) {
    }
}

/*
 * 测试用例： 批量查询资源
 * 前置条件： 应用实例创建成功
 * CHECK点： 能正确返回查询到的资源个数
 */
TEST_F(PluginMainTest, ListApplicationResourceV2Exists)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_ListApplicationResourceV2_exists);
    ResourceResultByPage page {};
    ListResourceRequest request {};
    ListApplicationResourceV2(page, request);
    EXPECT_EQ(page.items.size(), 1);
}

/*
 * 测试用例： 批量查询资源参数校验失败
 * 前置条件： 应用实例创建成功
 * CHECK点： ListResourceRequest参数校验
 */
TEST_F(PluginMainTest, ListApplicationResourceV2ParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_ListApplicationResourceV2_exists);
    ResourceResultByPage page {};
    ListResourceRequest request {};
    QueryByPage condition {};
    condition.pageSize = 1001;
    request.condition = condition;
    try {
        ListApplicationResourceV2(page, request);  
    } catch (...) {
    }
}

/*
 * 测试用例： 批量查询主机集群，集群存在
 * 前置条件： 应用实例创建成功
 * CHECK点： 能正确返回查询到的集群个数
 */
TEST_F(PluginMainTest, DiscoverHostClusterExists)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_DiscoverHostCluster_exists);
    ApplicationEnvironment returnEnv {};
    ApplicationEnvironment appEnv {};
    DiscoverHostCluster(returnEnv, appEnv);
    EXPECT_EQ(returnEnv.nodes.size(), 1);
}

/*
 * 测试用例： 批量查询主机集群参数校验失败
 * 前置条件： 应用实例创建成功
 * CHECK点： ApplicationEnvironment参数校验
 */
TEST_F(PluginMainTest, DiscoverHostClusterParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_DiscoverHostCluster_exists);
    ApplicationEnvironment returnEnv {};
    ApplicationEnvironment appEnv {};
    appEnv.endpoint = "1.*.1.1";
    try {
        DiscoverHostCluster(returnEnv, appEnv);
    } catch (...) {
    }
}

/*
 * 测试用例： 批量查询应用集群，集群存在
 * 前置条件： 应用实例创建成功
 * CHECK点： 能正确返回查询到的集群个数
 */
TEST_F(PluginMainTest, DiscoverAppClusterExists)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_DiscoverAppCluster_exists);
    ApplicationEnvironment returnEnv {};
    ApplicationEnvironment appEnv {};
    Application application {};
    DiscoverAppCluster(returnEnv, appEnv, application);
    EXPECT_EQ(returnEnv.nodes.size(), 1);
}

/*
 * 测试用例： 批量查询应用集群参数校验失败
 * 前置条件： 应用实例创建成功
 * CHECK点： Application参数校验
 */
TEST_F(PluginMainTest, DiscoverAppClusterParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_DiscoverAppCluster_exists);
    ApplicationEnvironment returnEnv {};
    ApplicationEnvironment appEnv {};
    Application application {};
    AppProtect::Authentication auth {};
    auth.authType = static_cast<AppProtect::AuthType>(101);
    application.auth = auth;
    try {
        DiscoverAppCluster(returnEnv, appEnv, application);
    } catch (...) {
    }
}

/*
 * 用例名称：DME下发参数为全量备份时，执行全量备份
 * 前置条件：无
 * check点：解析参数为全量备份时，返回成功
 */
TEST_F(PluginMainTest, CheckBackupJobTypeFull)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(SUCCESS, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：BackupJob参数校验
 */
TEST_F(PluginMainTest, CheckBackupJobTypeParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::Copy copy {};
    copy.name = "copy@name";
    job.copy = copy;
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：COMMON_VALUE_10000规则校验
 */
TEST_F(PluginMainTest, CommonValue10000ParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::StorageRepository repository {};
    AppProtect::HostAddress remoteHost {};
    remoteHost.supportProtocol = 10001;
    repository.remoteHost = remoteHost;
    job.repositories = { repository };
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：COMMON_PATH规则校验
 */
TEST_F(PluginMainTest, CommonPathParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::StorageRepository repository {};
    repository.path ="1.@.%.1";
    job.repositories = { repository };
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：SCRIPT_PATH规则校验
 */
TEST_F(PluginMainTest, ScriptPathParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::BackupJobParam jobParam {};
    AppProtect::JobScripts scripts;
    scripts.preScript = "@path";
    job.jobParam = jobParam;
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：TIMESTAMP规则校验
 */
TEST_F(PluginMainTest, TimestampParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::Copy copy {};
    copy.timestamp = 10000000000000;
    job.copy = copy;
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：CheckBackupJobType参数校验失败
 * 前置条件：无
 * check点：SNAPSHOT_ID规则校验
 */
TEST_F(PluginMainTest, SnapshotIdParamCheckFailed)
{
    Stub stub;
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    AppProtect::Copy copy {};
    AppProtect::Snapshot snapshot{};
    snapshot.id = "parent.name";
    copy.snapshots = { snapshot };
    job.copy = copy;
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(0x4003291A, returnValue.code);
}

/*
 * 用例名称：DME下发参数为增量备份时，检查是否需要增量转全量
 * 前置条件：无
 * check点：插件无法执行增备时，返回特定错误码
 */
TEST_F(PluginMainTest, CheckBackupJobTypeIncToFull)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_CheckBackupJobType_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    FormCopyData(job);
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(INNER_ERROR, returnValue.code);
    EXPECT_EQ(BACKUP_INC_TO_FULL, returnValue.bodyErr);
}

/*
 * 用例名称：DME下发参数为增量备份时，检查是否需要增量转全量
 * 前置条件：无
 * check点：插件能法执行增备时，返回成功
 */
TEST_F(PluginMainTest, CheckBackupJobTypeInc)
{
    Stub stub;
    stub.set(ADDR(EngineFactory, CreateProtectEngineWithoutTask), Stub_CreateEngine_CheckBackupJobType_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    // stub.set(ADDR(HCSProtectEngine, GetStorageType), Stub_HcsGetStorageType);
    ActionResult returnValue {};
    AppProtect::BackupJob job {};
    job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    FormCopyData(job);
    CheckBackupJobType(returnValue, job);
    EXPECT_EQ(SUCCESS, returnValue.code);
}

/*
 * 用例名称：HCS恢复检查是否可以在本节点，返回成功
 * 前置条件：本节点与HCS生产环境连通
 * check点：AllowRestoreInLocalNode返回成功
 */
TEST_F(PluginMainTest, HCSCheckCanRestoreInLocalNodeSucc)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(HcsPlugin::HcsResourceAccess, CheckStorageConnect), Stub_CheckHcsSuccess);
    ActionResult returnValue {};
    AppProtect::RestoreJob job;
    SetEnvObj(job.targetEnv);
    job.targetEnv.subType = ENV_APPTYPE_HCS;
    AllowRestoreInLocalNode(returnValue, job);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：AllowRestoreInLocalNode参数校验失败
 * 前置条件：本节点与HCS生产环境连通
 * check点：RestoreJob参数校验
 */
TEST_F(PluginMainTest, AllowRestoreInLocalNodeParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(HcsPlugin::HcsResourceAccess, CheckStorageConnect), Stub_CheckHcsSuccess);
    ActionResult returnValue {};
    AppProtect::RestoreJob job;
    SetEnvObj(job.targetEnv);
    job.targetEnv.subType = ENV_APPTYPE_HCS;
    AppProtect::Copy copy {};
    AppProtect::Snapshot snapshot{};
    snapshot.parentName = "parent@name";
    copy.snapshots = { snapshot };
    job.copy = copy;
    AllowRestoreInLocalNode(returnValue, job);
    EXPECT_EQ(returnValue.code, 0x4003291A);
}

/*
 * 用例名称：HCS恢复检查是否可以在本节点，返回失败
 * 前置条件：本节点与HCS生产环境不连通
 * check点：AllowRestoreInLocalNode返回失败
 */
TEST_F(PluginMainTest, HCSCheckCanRestoreInLocalNodeFail)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailFail);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(HcsPlugin::HcsResourceAccess, CheckStorageConnect), Stub_CheckHcsSuccess);
    ActionResult returnValue {};
    AppProtect::RestoreJob job;
    SetEnvObj(job.targetEnv);
    job.targetEnv.subType = ENV_APPTYPE_HCS;
    AllowRestoreInLocalNode(returnValue, job);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

/*
 * 用例名称：HCS备份检查是否可以在本节点，返回成功
 * 前置条件：本节点与HCS生产环境连通
 * check点：AllowBackupInLocalNode返回成功
 */
TEST_F(PluginMainTest, HCSCheckCanBackupInLocalNodeSucc)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(HcsPlugin::HcsResourceAccess, CheckStorageConnect), Stub_CheckHcsSuccess);
    ActionResult returnValue {};
    AppProtect::BackupJob job;
    AppProtect::BackupLimit::type limit;
    SetEnvObj(job.protectEnv);
    job.protectEnv.subType = ENV_APPTYPE_HCS;
    AllowBackupInLocalNode(returnValue, job, limit);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：AllowBackupInLocalNode参数校验失败
 * 前置条件：本节点与HCS生产环境连通
 * check点：BackupJob参数校验
 */
TEST_F(PluginMainTest, AllowBackupInLocalNodeParamCheckFailed)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(HcsPlugin::HcsResourceAccess, CheckStorageConnect), Stub_CheckHcsSuccess);
    ActionResult returnValue {};
    AppProtect::BackupJob job;
    AppProtect::BackupLimit::type limit;
    SetEnvObj(job.protectEnv);
    job.protectEnv.subType = ENV_APPTYPE_HCS;
    BackupJobParam jobParam {};
    ResourceFilter filters {};
    filters.mode = "mode";
    jobParam.filters = filters;
    job.jobParam = jobParam;
    AllowBackupInLocalNode(returnValue, job, limit);
    EXPECT_EQ(returnValue.code, 0x4003291A);
}

/*
 * 用例名称：HCS备份检查是否可以在本节点，返回失败
 * 前置条件：本节点与HCS生产环境不连通
 * check点：AllowBackupInLocalNode返回失败
 */
TEST_F(PluginMainTest, HCSCheckCanBackupInLocalNodeFail)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailFail);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    ActionResult returnValue {};
    AppProtect::BackupJob job;
    AppProtect::BackupLimit::type limit;
    SetEnvObj(job.protectEnv);
    job.protectEnv.subType = ENV_APPTYPE_HCS;
    AllowBackupInLocalNode(returnValue, job, limit);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

/*
 * 用例名称：分布式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：分布式存储 AllowBackupSubJobInLocalNode返回成功
 */
TEST_F(PluginMainTest, FusionStorHCSCheckBackupSubJobInLocalNodeSucc)
{
    Stub stub;
    typedef int32_t (*fptr)(VirtPlugin::FusionStorageVolumeHandler*);
    fptr Api_TestDeviceConnection = (fptr)(&VirtPlugin::FusionStorageVolumeHandler::TestDeviceConnection);
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(Api_TestDeviceConnection, Stub_FusionStorage_TestDeviceConnection_Success);

    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    int32_t errorCode = SUCCESS;
    AllowBackupSubJobInLocalNode(returnValue, m_backupJobInfo, m_backupSubJob, errorCode);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：AllowBackupSubJobInLocalNode参数校验失败
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：SubJob参数校验
 */
TEST_F(PluginMainTest, AllowBackupSubJobInLocalNodeParamCheckFailed)
{
    Stub stub;
    typedef int32_t (*fptr)(VirtPlugin::FusionStorageVolumeHandler*);
    fptr Api_TestDeviceConnection = (fptr)(&VirtPlugin::FusionStorageVolumeHandler::TestDeviceConnection);
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(Api_TestDeviceConnection, Stub_FusionStorage_TestDeviceConnection_Success);

    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    Subjob subJob {};
    subJob.jobName = "*jobName";
    int32_t errorCode = SUCCESS;
    AllowBackupSubJobInLocalNode(returnValue, m_backupJobInfo, subJob, errorCode);
    EXPECT_EQ(returnValue.code, 0x4003291A);
}

/*
 * 用例名称：分布式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储不通
 * check点：分布式存储 AllowBackupSubJobInLocalNode返回失败
 */
TEST_F(PluginMainTest, FusionStorHCSCheckBackupSubJobInLocalNodeFailed)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(VirtPlugin::FusionStorageVolumeHandler, CheckHealthStatus),
        Stub_FusionStorage_CheckHealthStatus_Failed);
    stub.set(ADDR(VirtPlugin::FusionStorageRestApiOperator, GetCurrentSession),
        Stub_FusionStorageRestApiOperator_GetCurrentSession_Failed);
    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    int32_t errorCode = SUCCESS;
    AllowBackupSubJobInLocalNode(returnValue, m_backupJobInfo, m_backupSubJob, errorCode);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

/*
 * 用例名称：集中式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：集中式存储 AllowBackupSubJobInLocalNode返回成功
 */
TEST_F(PluginMainTest, OceanStorHCSCheckBackupSubJobInLocalNodeSucc)
{
    typedef int32_t (*fptr)(VirtPlugin::ApiOperator*);
    fptr ApiOperator_TestDeviceConnection = (fptr)(&VirtPlugin::ApiOperator::TestDeviceConnection);
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ApiOperator_TestDeviceConnection, Stub_OceanStor_TestDeviceConnection_Success);
    ActionResult returnValue {};
    FormJobInfo("OceanStorV5");
    int32_t errorCode = SUCCESS;
    AllowBackupSubJobInLocalNode(returnValue, m_backupJobInfo, m_backupSubJob, errorCode);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：集中式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储不通
 * check点：集中式存储 AllowBackupSubJobInLocalNode返回失败
 */
TEST_F(PluginMainTest, OceanStorHCSCheckBackupSubJobInLocalNodeFailed)
{
    typedef int32_t (*fptr)(VirtPlugin::ApiOperator*);
    fptr ApiOperator_TestDeviceConnection = (fptr)(&VirtPlugin::ApiOperator::TestDeviceConnection);
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ApiOperator_TestDeviceConnection, Stub_OceanStor_TestDeviceConnection_Failed);
    ActionResult returnValue {};
    FormJobInfo("OceanStorV5");
    int32_t errorCode = SUCCESS;
    AllowBackupSubJobInLocalNode(returnValue, m_backupJobInfo, m_backupSubJob, errorCode);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

/*
 * 用例名称：分布式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：分布式存储 AllowRestoreSubJobInLocalNode 返回成功
 */
TEST_F(PluginMainTest, FusionStorHCSCheckRestoreSubJobInLocalNodeSucc)
{
    Stub stub;
    typedef int32_t (*fptr)(VirtPlugin::FusionStorageVolumeHandler*);
    fptr Api_TestDeviceConnection = (fptr)(&VirtPlugin::FusionStorageVolumeHandler::TestDeviceConnection);
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(Api_TestDeviceConnection, Stub_FusionStorage_TestDeviceConnection_Success);

    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    AllowRestoreSubJobInLocalNode(returnValue, m_restoreJobInfo, m_restoreSubJob);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：AllowRestoreSubJobInLocalNode参数校验失败
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：SubJob参数校验
 */
TEST_F(PluginMainTest, AllowRestoreSubJobInLocalNodeParamCheckFailed)
{
    Stub stub;
    typedef int32_t (*fptr)(VirtPlugin::FusionStorageVolumeHandler*);
    fptr Api_TestDeviceConnection = (fptr)(&VirtPlugin::FusionStorageVolumeHandler::TestDeviceConnection);
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(Api_TestDeviceConnection, Stub_FusionStorage_TestDeviceConnection_Success);

    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    Subjob subJob {};
    subJob.jobPriority = 101;
    AllowRestoreSubJobInLocalNode(returnValue, m_restoreJobInfo, m_restoreSubJob);
    EXPECT_EQ(returnValue.code, 0x4003291A);
}

/*
 * 用例名称：分布式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储不通
 * check点：分布式存储 AllowRestoreSubJobInLocalNode 返回失败
 */
TEST_F(PluginMainTest, FusionStorHCSCheckRestoreSubJobInLocalNodeFailed)
{
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ADDR(VirtPlugin::FusionStorageVolumeHandler, CheckHealthStatus),
        Stub_FusionStorage_CheckHealthStatus_Failed);
    stub.set(ADDR(VirtPlugin::FusionStorageRestApiOperator, GetCurrentSession),
        Stub_FusionStorageRestApiOperator_GetCurrentSession_Failed);
    ActionResult returnValue {};
    FormJobInfo("FusionStorage");
    AllowRestoreSubJobInLocalNode(returnValue, m_restoreJobInfo, m_restoreSubJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}

/*
 * 用例名称：集中式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储连通
 * check点：集中式存储 AllowRestoreSubJobInLocalNode 返回成功
 */
TEST_F(PluginMainTest, OceanStorHCSCheckRestoreSubJobInLocalNodeSucc)
{
    typedef int32_t (*fptr)(ApiOperator*);
    fptr ApiOperator_TestDeviceConnection = (fptr)(&ApiOperator::TestDeviceConnection);
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ApiOperator_TestDeviceConnection, Stub_OceanStor_TestDeviceConnection_Success);
    ActionResult returnValue {};
    FormJobInfo("OceanStorV5");
    AllowRestoreSubJobInLocalNode(returnValue, m_restoreJobInfo, m_restoreSubJob);
    EXPECT_EQ(returnValue.code, SUCCESS);
}

/*
 * 用例名称：集中式存储，HCS备份子任务检查是否可以在本节点执行
 * 前置条件：本节点与HCS生产环境连通、与生产存储不通
 * check点：集中式存储 AllowRestoreSubJobInLocalNode 返回失败
 */
TEST_F(PluginMainTest, OceanStorHCSCheckRestoreSubJobInLocalNodeFailed)
{
    typedef int32_t (*fptr)(ApiOperator*);
    fptr ApiOperator_TestDeviceConnection = (fptr)(&ApiOperator::TestDeviceConnection);
    Stub stub;
    stub.set(ADDR(VirtPlugin::HttpClient, Send), Stub_GetServerDetailSuccess);
    stub.set(ADDR(HcsPlugin::HCSTokenMgr, GetToken), Stub_TokenSuccess);
    stub.set(ApiOperator_TestDeviceConnection, Stub_OceanStor_TestDeviceConnection_Failed);
    ActionResult returnValue {};
    FormJobInfo("OceanStorV5");
    AllowRestoreSubJobInLocalNode(returnValue, m_restoreJobInfo, m_restoreSubJob);
    EXPECT_EQ(returnValue.code, INNER_ERROR);
}
}
