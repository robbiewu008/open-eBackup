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
#include "taskmanager/PluginMainJobTest.h"
#include <memory>
#include <thread>
#include <vector>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransport.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "common/JsonUtils.h"
#include "common/Semaphore.h"
#include "securecom/RootCaller.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "message/curlclient/DmeRestClient.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "thriftservice/include/IThriftService.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "message/curlclient/HttpClientInterface.h"
#include "taskmanager/externaljob/ExecutorBuilder.h"
#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "taskmanager/externaljob/PluginSubPostJob.h"
#include "taskmanager/externaljob/PluginSubBusiJob.h"
#include "taskmanager/externaljob/PluginSubGeneJob.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "servicecenter/services/device/CacheRepository.h"
#include "servicecenter/services/device/RepositoryFactory.h"
#include "host/host.h"

using namespace servicecenter;
using namespace thriftservice;
using namespace AppProtect;
namespace {
static mp_bool g_execFlag = MP_FALSE;
mp_void LogTest()
{}

#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

class ProtectServiceClientNoAllowTest : public ProtectServiceClient {
public:
    ProtectServiceClientNoAllowTest(std::shared_ptr<::apache::thrift::protocol::TProtocol> prot)
        : ProtectServiceClient(prot)
    {}
    ProtectServiceClientNoAllowTest(std::shared_ptr<::apache::thrift::protocol::TProtocol> iprot,
        std::shared_ptr<::apache::thrift::protocol::TProtocol> oprot)
        : ProtectServiceClient(iprot, oprot)
    {}
    ~ProtectServiceClientNoAllowTest()
    {}

    void AllowBackupInLocalNode(ActionResult& _return, const BackupJob& job, const BackupLimit::type limit) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_FAILED;
    }

    void AllowRestoreInLocalNode(ActionResult& _return, const RestoreJob& job) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_FAILED;
    }
};

static std::string excutorResult;

class ProtectServiceClientAllowTest : public ProtectServiceClient {
public:
    ProtectServiceClientAllowTest(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}

    void CheckBackupJobType(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_SUCCESS;
    }

    void AllowBackupInLocalNode(ActionResult& _return, const BackupJob& job, const BackupLimit::type limit) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_SUCCESS;
    }

    void AllowRestoreInLocalNode(ActionResult& _return, const RestoreJob& job) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_SUCCESS;
    }

    void AllowBackupSubJobInLocalNode(ActionResult& _return, const BackupJob& job, const SubJob& subJob) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_SUCCESS;
    }

    void AllowRestoreSubJobInLocalNode(ActionResult& _return, const RestoreJob& job, const SubJob& subJob) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_SUCCESS;
    }

    void AsyncRestorePrerequisite(ActionResult& _return, const RestoreJob& job) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncBackupPrerequisite(ActionResult& _return, const BackupJob& job) override
    {
        excutorResult += "AsyncBackupPrerequisite Success ";
        _return.code = MP_SUCCESS;
    }

    void AsyncBackupGenerateSubJob(ActionResult& _return, const BackupJob& job, const int32_t nodeNum) override
    {
        excutorResult += "AsyncBackupGenerateSubJob Success ";
        _return.code = MP_SUCCESS;
    }

    void AsyncRestoreGenerateSubJob(ActionResult& _return, const RestoreJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncExecuteBackupSubJob(ActionResult& _return, const BackupJob& job, const SubJob& subjob) override
    {
        excutorResult += "AsyncExecuteBackupSubJob Success ";
        _return.code = MP_SUCCESS;
    }

    void AsyncBackupPostJob(ActionResult& _return, const BackupJob& job, const SubJob& subJob, const JobResult::type backupJobResult) override
    {
        excutorResult += "AsyncBackupPostJob Success ";
        _return.code = MP_SUCCESS;
    }

};

class ProtectServiceClientCallFailedTest : public ProtectServiceClient {
public:
    ProtectServiceClientCallFailedTest(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}

    void CheckBackupJobType(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_FAILED;
    }

    void AllowBackupInLocalNode(ActionResult& _return, const BackupJob& job, const BackupLimit::type limit) override
    {
        g_execFlag = MP_TRUE;
        _return.code = MP_FAILED;
    }

    void AsyncRestorePrerequisite(ActionResult& _return, const RestoreJob& job) override
    {
        _return.code = MP_FAILED;
    }

    void AsyncBackupPrerequisite(ActionResult& _return, const BackupJob& job) override
    {
        excutorResult += "AsyncBackupPrerequisite Failed ";
        _return.code = MP_FAILED;
    }

    void AsyncBackupGenerateSubJob(ActionResult& _return, const BackupJob& job, const int32_t nodeNum) override
    {
        excutorResult += "AsyncBackupGenerateSubJob Failed ";
        _return.code = MP_FAILED;
    }

    void AsyncRestoreGenerateSubJob(ActionResult& _return, const RestoreJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_FAILED;
    }
    void AsyncExecuteBackupSubJob(ActionResult& _return, const BackupJob& job, const SubJob& subjob) override
    {
        excutorResult += "AsyncExecuteBackupSubJob Failed ";
        _return.code = MP_FAILED;
    }

    void AsyncBackupPostJob(ActionResult& _return, const BackupJob& job, const SubJob& subJob, const JobResult::type backupJobResult) override
    {
        excutorResult += "AsyncBackupPostJob Failed ";
        _return.code = MP_FAILED;
    }
};

mp_int32 StubExistJob(const mp_string& mainId, struct PluginJobData& mainJob)
{
    return MP_SUCCESS;
}
mp_int32 StubNoExistJob(const mp_string& mainId, struct PluginJobData& mainJob)
{
    return MP_NOEXISTS;
}
mp_int32 StubInsertMainJob(const struct PluginJobData& info)
{
    g_execFlag = MP_TRUE;
    return MP_SUCCESS;
}

mp_int32 StubInsertMainJobFailed(const struct PluginJobData& info)
{
    g_execFlag = MP_TRUE;
    return MP_FAILED;
}

mp_uint32 StubGetHttpStatusCode()
{
    return 500;
}

mp_int32 StubRestSendResponseFailed(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value rspValue;
    rspValue["errorCode"] = "1";
    httpResponse.body = rspValue.toStyledString();
    return MP_FAILED;
}

static bool g_call = false;
mp_int32 StubRestSendResponse(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value rspValue;
    rspValue["errorCode"] = "0";
    httpResponse.body = rspValue.toStyledString();
    g_call = true;
    return MP_SUCCESS;
}

mp_int32 StubGetUbcIpsByMainJobIdSuccess(mp_void* pThis, const mp_string mainJobId, std::vector<mp_string>& ubcIps)
{
    return MP_SUCCESS;
}

mp_int32 StubJobConvertStringtoJsonSuccess(void *obj, const mp_string& rawBuffer, Json::Value& jsValue)
{
    jsValue["errorCode"] = "0";
    Json::Value jsResultValue;
    jsResultValue["failed"] = 0;
    jsResultValue["aborted"] = 0;
    jsValue["subTask"] = jsResultValue;
    return MP_SUCCESS;
}

mp_int32 StubUpdateSecureInfoOK()
{
    return MP_SUCCESS;
}

mp_int32 StubCheckIsDoradoEnvironment(void *obj, mp_bool& isDorado)
{
    isDorado = MP_FALSE;
    return MP_SUCCESS;
}

std::shared_ptr<thriftservice::IThriftClient> StubGetThriftClient()
{
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "127.0.0.1", 59610 };
    std::shared_ptr<thriftservice::IThriftClient> thriftclient = thriftservice->RegisterClient(opt);
    return thriftclient;
}

std::shared_ptr<ProtectServiceClient> StubNoAllowGetThriftClient(mp_void* pThis, std::shared_ptr<thriftservice::IThriftClient> pThriftClient)
{
    std::shared_ptr<::apache::thrift::transport::TTransport> socket =
        std::make_shared<::apache::thrift::transport::TSocket>("127.0.0.1", 10000);
    std::shared_ptr<::apache::thrift::transport::TTransport> transport =
        std::make_shared<::apache::thrift::transport::TFramedTransport>(socket);
    std::shared_ptr<::apache::thrift::protocol::TProtocol> protocol =
        std::make_shared<::apache::thrift::protocol::TBinaryProtocol>(transport);
    return std::make_shared<ProtectServiceClientNoAllowTest>(protocol);
}

static std::shared_ptr<ProtectServiceIf> g_ProtectServiceClient;
static std::shared_ptr<ProtectServiceIf> StubAllowGetThriftClient(mp_void* pThis, std::shared_ptr<thriftservice::IThriftClient> pThriftClient)
{
    if (g_ProtectServiceClient.get() == nullptr) {
        g_ProtectServiceClient = std::make_shared<ProtectServiceClientAllowTest>(nullptr);
    }
    return g_ProtectServiceClient;
}

static std::shared_ptr<ProtectServiceClient> g_ProtectServiceFailedClient;
static std::shared_ptr<ProtectServiceClient> StubAllowGetThriftClientAndPluginReturnFailed()
{
    if (g_ProtectServiceFailedClient.get() == nullptr) {
        g_ProtectServiceFailedClient = std::make_shared<ProtectServiceClientCallFailedTest>(nullptr);
    }
    return g_ProtectServiceFailedClient;
}

mp_int32 StubUpdateJobStatusInfoFailed(const mp_string& mainId, const mp_string& subId, mp_uint32 status)
{
    g_execFlag = MP_TRUE;
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubJobExecSuccess(void* obj) 
{
    return MP_SUCCESS;
}

mp_int32 StubJobExecFailed(void* obj) 
{
    return MP_FAILED;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

void StubTestRunJobThread(std::shared_ptr<Job> ptr)
{
    ptr->Exec();
}

mp_int32 StubUpdateJobStatusInfo(const mp_string& mainId, mp_int32 state)
{
    return MP_SUCCESS;
}

mp_int32 StubSendRequestSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    return MP_SUCCESS;
}

mp_int32 StubConvertStringtoJsonSuccess(void *obj, const mp_string& rawBuffer, Json::Value& jsValue)
{
    jsValue["errorCode"] = "0";
    return MP_SUCCESS;
}

mp_int32 SendAbortToPluginSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubMountNasFileSystem(void* obj, MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, 
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const MountPermission &permit)
{
    successMountPath.push_back("/mnt/databackup/pluginName/mainJobID/data/Share1/192.168.145.161");
    return MP_SUCCESS;
}

static std::shared_ptr<Repository> g_pRepository;
std::shared_ptr<Repository> StubCreateRepository(void* obj, RepositoryDataType::type repositoryType)
{
    g_pRepository = std::make_shared<AppProtect::Repository>();
    return g_pRepository;
}

mp_int32 StubMount(PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    return MP_SUCCESS;
}

mp_int32 StubAssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    return MP_SUCCESS;
}

static bool g_waitSuccessCall = false;
mp_int32 WaitPluginNotifySuccessStub(void* obj) {
    excutorResult += "WaitPluginNotify Success ";
    g_waitSuccessCall = true;
    return MP_SUCCESS;
}

static bool g_waitFailedCall = false;
mp_int32 WaitPluginNotifyFailedStub(void* obj) {
    excutorResult += "WaitPluginNotify Failed ";
    g_waitFailedCall = true;
    return MP_FAILED;
}

mp_int32 MountNasSuccessStub()
{
    excutorResult += "MountNas Success ";
    return MP_SUCCESS;
}

mp_int32 MountNasFailedStub()
{
    excutorResult += "MountNas Failed ";
    return MP_FAILED;
}

mp_int32 UnmountNasSuccessStub()
{
    excutorResult += "UnmountNas Success ";
    return MP_SUCCESS;
}

mp_int32 JobSplitRepositoriesSub(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 StubGetJobsExecResult(void* obj)
{
    return MP_SUCCESS;
}

static AppProtect::SubJobDetails g_jobDetails;
static mp_int32 g_stage;
static std::string g_label;
mp_int32 StubSendDetailToDme(void* obj, const AppProtect::SubJobDetails& jobInfo, mp_int32 jobStage)
{
    g_jobDetails = jobInfo;
    g_stage = jobStage;
    g_call = true;
    if (g_jobDetails.logDetail.size() == 1) {
        g_label = g_jobDetails.logDetail[0].description;
    }
    return MP_SUCCESS;
}

mp_int32 StubInitialize(void* obj) 
{
    return MP_SUCCESS;
}

mp_int32 StubGetHostSNSuccess(void *obj, mp_string& strSN)
{
    strSN = "123456";
    return MP_SUCCESS;
}

mp_int32 StubExecAllowBackupInLocalNode(void *obj, const BackupLimit::type policy)
{
    return MP_SUCCESS;
}
}  // namespace

/*
 * 用例名称：主任务初始化成功
 * 前置条件：生成的任务进行初始化
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginMainJobTest, InitializeTest)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mp_int32 iRet = pluginManJob.get()->Initialize();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：主任务已完成时重新执行
 * 前置条件：生成的任务已完成，重新执行
 * check点：1、Mock log 2、主任务运行成功
 */
TEST_F(PluginMainJobTest, MainJobCompleteTest)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    EXPECT_EQ(MP_SUCCESS, pluginManJob.get()->Initialize());
    pluginManJob->m_data.status = mp_uint32(MainJobState::COMPLETE);
    EXPECT_EQ(MP_SUCCESS, pluginManJob.get()->Exec());
}

/*
 * 用例名称：主任务状态不满足要求时重新执行
 * 前置条件：生成的任务不满足，重新执行
 * check点：1、Mock log 2、主任务运行失败
 */
TEST_F(PluginMainJobTest, MainJobUnDefinedTest)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mp_int32 iRet = pluginManJob.get()->Initialize();
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(MP_SUCCESS, pluginManJob.get()->Exec());
}

/*
 * 用例名称：不存在主任务，初始化新增主任务成功
 * 前置条件：mock数据库中没有主任务，然后初始化
 * check点：1、Mock log 2、mock DB没有主任务 3、mock 添加新的主任务
 */
TEST_F(PluginMainJobTest, InitializeNoExistJobOKTest)
{
    g_execFlag = MP_FALSE;
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubNoExistJob);
    stub.set(&JobStateDB::InsertRecord, StubInsertMainJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&DmeRestClient::UpdateSecureInfo, StubUpdateSecureInfoOK);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mp_int32 iRet = pluginManJob.get()->Initialize();
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(g_execFlag, MP_TRUE);
}

/*
 * 用例名称：不存在主任务，初始化新增主任务失败
 * 前置条件：mock数据库中没有主任务，然后初始化
 * check点：1、Mock log 2、mock DB没有主任务 3、添加新的主任务
 */
TEST_F(PluginMainJobTest, InitializeNoExistJobFailedTest)
{
    g_execFlag = MP_FALSE;
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubNoExistJob);
    stub.set(&JobStateDB::InsertRecord, StubInsertMainJobFailed);
    stub.set(&DmeRestClient::UpdateSecureInfo, StubUpdateSecureInfoOK);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mp_int32 iRet = pluginManJob.get()->Initialize();
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(g_execFlag, MP_TRUE);
}

/*
 * 用例名称：测试CanbeRunInLocalNode
 * 前置条件：NA
 * check点：1.当前节点不允许执行，返回失败 
                    2.允许执行，更新状态失败，返回失败
                    3.允许执行，更新状态成功，返回成功
 */
TEST_F(PluginMainJobTest, CanbeRunInLocalNode)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pBackupJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubNoAllowGetThriftClient);
    EXPECT_EQ(ERR_PLUGIN_AUTHENTICATION_FAILED, pBackupJob->CanbeRunInLocalNode());
    
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    EXPECT_EQ(MP_SUCCESS, pBackupJob->CanbeRunInLocalNode());

    PluginJobData rJobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::RESTORE_JOB};
    std::shared_ptr<Job> pRestoreJob = PluginJobFactory::GetInstance()->CreatePluginJob(rJobData);

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubNoAllowGetThriftClient);
    EXPECT_EQ(ERR_PLUGIN_AUTHENTICATION_FAILED, pRestoreJob->CanbeRunInLocalNode());
    
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    EXPECT_EQ(MP_SUCCESS, pRestoreJob->CanbeRunInLocalNode());
}

/*
 * 用例名称：当前节点允许该任务，调用DME rest， return failed
 * 前置条件：NA
 * check点：1、Mock log 2、mock Send message success and response have error code 3、返回失败 4、未调用设置任务状态
 */
TEST_F(PluginMainJobTest, GenerateSendResponseFailedTest)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponseFailed);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_EQ(MP_SUCCESS, pJob->Initialize());
    pJob->Exec();
    EXPECT_EQ(static_cast<int32_t>(MainJobState::FAILED), pJob->GetData().status);
}

/*
 * 用例名称：当前节点允许该任务，调用DME rest成功，但是更新DB状态失败
 * 前置条件：NA
 * check点：1、Mock log 2、mock Send message success and response success 3、mock设置任务状态 failed 4、返回失败
 */
TEST_F(PluginMainJobTest, GenerateUpdateJobStatusFailedTest)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&DmeRestClient::SendRequest, StubSuccess);
    
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_EQ(MP_SUCCESS, pJob->Initialize());
    stub.set(&JobStateDB::UpdateStatus, StubUpdateJobStatusInfoFailed);
    pJob->Exec();
    EXPECT_EQ(static_cast<int32_t>(MainJobState::FAILED), pJob->GetData().status);
}

/*
 * 用例名称：步骤“校验备份类型”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock GetProtectServiceClient 4.mock 除CheckBackupJobType外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, CheckBackupJobType_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreSubJob, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Exec());
}

/*
 * 用例名称：步骤“执行前置子任务”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock GetProtectServiceClient 4.mock 除ExecutePreSubJob外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, ExecutePreSubJob_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Exec());
}

/*
 * 用例名称：步骤“执行前置脚本”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock GetProtectServiceClient 4.mock 除ExecutePreScript外的其他步骤 5.mock CRootCaller
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, ExecutePreScript_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreSubJob, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    Json::Value paramScript;
    paramScript[KEY_PRE_SCRIPTS] = "test.sh";
    Json::Value param;
    param[KEY_SCRIPTS] = paramScript;
    PluginJobData jobData = {"pluginName", "mainJobID", "", param,  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Exec());
}

/*
 * 用例名称：步骤“执行分解子任务”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock GetProtectServiceClient 4.mock 除ExecGenerateSubJob外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, ExecGenerateSubJob_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreSubJob, StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
 * 用例名称：步骤“执行nas文件系统挂载”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock 除MountNas外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, MountNas_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreSubJob, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(&Repository::GetMountIP, StubSuccess);
    stub.set(&PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(&RepositoryFactory::CreateRepository, StubCreateRepository);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    mp_string strValue = "{ \"repositories\" : [{ \"type\" : 1, \"protocol\" : \
        1, \"remoteHost\" : [{ \"ip\" : \"192.168.145.161\" } ], \"remotePath\" : \"/Share1\" } ] }";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData jobData = {"pluginName", "mainJobID", "", jsValue,  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
 * 用例名称：步骤“拆分repositories字段成功”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock 除MountNas外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, SplitRepositories_Success)
{
    DoLogTest();
    mp_string strValue = "{ \"repositories\" : [{\"role\" : 0, \"remotePath\" : [{ \"type\" : 1, \"path\" : \"path1\", \
        \"remoteHost\" : [{ \"ip\" : \"192.168.1.1\", \"port\" : 1000}], \"id\" : \"repositoryId1\"}, \
        { \"type\" : 0, \"path\" : \"path2\", \"remoteHost\" : [{ \"ip\" : \"192.168.1.2\", \"port\" : 1001}], \
        \"id\" : \"repositoryId2\"}} ] } ], \"copyInfo\" : { \"repositories\" : null } }";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", jsValue,  
        MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    pJob->SplitRepositories();
    EXPECT_EQ(2, pJob->GetData().param["repositories"].size());
}

/*
 * 用例名称：步骤“拆分repositories字段成功”成功
 * 前置条件：1.Mock log 2.mock QueryJob 3.mock 除MountNas外的其他步骤
 * check点：用PluginJobFactory创建一个mainJob，执行成功
 */
TEST_F(PluginMainJobTest, FilerUnvalidDoradoIps)
{
    DoLogTest();
    mp_string strValue = "{ \"repositories\" : [{ \"remoteHost\" : [{ \"id\" : null, \"ip\" : \"8.42.99.244\", \"port\" : null }, \
        { \"id\" : null, \"ip\" : \"8.42.99.246\", \"port\" : null }, { \"id\" : null, \"ip\" : \"8.42.99.212\", \"port\" : null } ] }] }";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", jsValue,  
        MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    std::vector<mp_string> validDoradoIps;
    validDoradoIps.push_back("8.42.99.212");
    pJob->FilerUnvalidDoradoIps(validDoradoIps);
    BackupJob jobParam;
    JsonToStruct(pJob->GetData().param, jobParam);
    EXPECT_EQ(1, jobParam.repositories[0].remoteHost.size());
    EXPECT_EQ("8.42.99.212", jobParam.repositories[0].remoteHost[0].ip);
}

/*
 * 用例名称：主任务不同阶段收到中止请求的不同处理
 * 前置条件：1. 在主任务正在执行
 * check点：主任务不同阶段收到中止请求的不同处理
 */
 TEST_F(PluginMainJobTest, AbortBeforePrejobRun)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob); 
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);

    /* 主任务在INITIALIZING时收到中止 */
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob->Initialize());
    pJob->Abort();
    EXPECT_EQ((mp_uint32)MainJobState::COMPLETE, pJob->GetData().status);
    /* 主任务在COMPLETE时收到中止 */
    pJob->Abort();
    EXPECT_EQ((mp_uint32)MainJobState::COMPLETE, pJob->GetData().status);
    /* 主任务在RUNNING时收到中止 */
    std::shared_ptr<Job> pJobRunning = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJobRunning.get());
    EXPECT_EQ(MP_SUCCESS, pJobRunning->Initialize());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubConvertStringtoJsonSuccess);
    stub.set(ADDR(Job, SendAbortToPlugin), SendAbortToPluginSuccess);

    EXPECT_EQ(MP_SUCCESS, pJobRunning->CanbeRunInLocalNode());
    std::thread t1(StubTestRunJobThread, pJobRunning);
    t1.detach();
    CMpTime::DoSleep(500);
    pJobRunning->Abort();
    EXPECT_EQ((mp_uint32)MainJobState::ABORTING, pJobRunning->GetData().status);
    /* 主任务收到中止完成 */
    AppProtect::SubJobDetails jobDetail;
    jobDetail.jobId = "mainJobID";
    jobDetail.jobStatus = SubJobStatus::type::ABORTED;
    pJobRunning->NotifyJobDetail(jobDetail);
    EXPECT_EQ((mp_uint32)MainJobState::COMPLETE, pJobRunning->GetData().status);
}

/*
 * 用例名称：主任务参数解析
 * 前置条件：1. 获取到dme发送的json字段
 * check点：dme的json转thrift结构体字段是否ok
 */
 TEST_F(PluginMainJobTest, Main_Job_param_parser_test_ok)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS","subType" : "HDFSFileset",},
	"taskParams" :{"backupType" : "fullBackup","copyFormat" : 0, "dataLayout":{"compression" : true,"deduption" : true,
    "encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,"scripts":{"failPostScript" : null,"postScript" : null,
    "preScript" : null}},"taskType" : 1})";
    BackupJob jobParam;
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    JsonToStruct(jsValue, jobParam);
    EXPECT_EQ(jobParam.jobParam.backupType, AppProtect::BackupJobType::FULL_BACKUP);
    EXPECT_EQ(jobParam.jobId, "5ec7b13d-9aac-4d03-adb6-f1e0f387081e");
    EXPECT_EQ(jobParam.protectObject.type, "HDFS");
    EXPECT_EQ(jobParam.protectObject.subType, "HDFSFileset");
    
}

/*
 * 用例名称：子任务参数解析
 * 前置条件：1. 获取到dme发送的json字段
 * check点：dme的json转thrift结构体字段是否ok
 */

 TEST_F(PluginMainJobTest, sub_Job_param_parser_test_ok)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "subTaskId" : "123344556780",
    "subTaskParams" : "{\"jobNo\": 0, \"path\": \"/tmp/huawei/databackup/HDFSFilesef_1.csv\"}"})";
    SubJob jobParam;
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    JsonToStruct(jsValue, jobParam);
    EXPECT_EQ(jobParam.jobInfo, "{\"jobNo\": 0, \"path\": \"/tmp/huawei/databackup/HDFSFilesef_1.csv\"}");
    EXPECT_EQ(jobParam.jobId, "5ec7b13d-9aac-4d03-adb6-f1e0f387081e");
    EXPECT_EQ(jobParam.subJobId, "123344556780");
}

/*
 * 用例名称：恢复任务参数解析
 * 前置条件：1. 获取到dme发送的json字段
 * check点：dme的json转thrift结构体字段是否ok
 */

 TEST_F(PluginMainJobTest, Restore_Job_param_parser_test_ok)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
        "repositories" :[{
                "path" : "",
                "protocol" : 1,
                "remoteHost" :[{"ip" : "8.42.99.247"},{"ip" : "8.42.99.244"}],
                "remotePath" :[{"path" : "/clone_HDFS_d09f64e4-3b62-42c5-b91e-a36288e0755d/source_policy_d09f64e4-3b62-42c5-b91e-a36288e0755d_Context_Global_MD",
                        "type" : 0},
                        {"path" : "/clone_HDFS_d09f64e4-3b62-42c5-b91e-a36288e0755d/source_policy_d09f64e4-3b62-42c5-b91e-a36288e0755d_Context",
                        "type" : 1}],
                "type" : 1}]
        }],
    "taskType" : 2
    })";
    RestoreJob jobParam;
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    JsonToStruct(jsValue, jobParam);
    EXPECT_EQ(jobParam.requestId, "742222ad-721e-4243-a410-1eab3691f9e3");
    EXPECT_EQ(jobParam.jobId, "5ec7b13d-9aac-4d03-adb6-f1e0f387081e");
    EXPECT_EQ(jobParam.jobParam.restoreType, AppProtect::RestoreJobType::NORMAL_RESTORE);
    EXPECT_EQ(jobParam.copies[0].protectSubObjects.size(), 1);
    EXPECT_EQ(jobParam.copies[0].protectSubObjects[0].name, "/liuxiaoxiang/happyFolder");
    EXPECT_EQ(jobParam.copies[0].repositories.size(), 1);
    EXPECT_EQ(jobParam.copies[0].repositories[0].remoteHost.size(), 2);
}

/*
 * 用例名称：主任务状态机处理函数构建
 * 前置条件：1. 获取到主任务
 * check点：任务状态机是否执行
 */
TEST_F(PluginMainJobTest, main_job_make_action_test)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);

    using mainJobAction = JobStateAction<MainJobState>;
    auto initAction = std::make_shared<mainJobAction>(
        "Initialing", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::CHECK_BACKUP_TYPE);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    auto action = pluginManJob->MakeAction([](){return StubSuccess();}, PluginMainJob::ActionEvent::JOB_START_EXEC);
    EXPECT_NE(action, nullptr);
    initAction->OnTransition = action;
    auto result = initAction->Transition();
    EXPECT_EQ(result, MainJobState::CHECK_BACKUP_TYPE);
}

/*
 * 用例名称：主任务状态机处理后置上报任务开始执行
 * 前置条件：1. 获取到主任务
 * check点：是否上报主任务进度
 */
TEST_F(PluginMainJobTest, main_job_report_task_start_label)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobIdSuccess);


    using mainJobAction = JobStateAction<MainJobState>;
    auto initAction = std::make_shared<mainJobAction>(
        "Initialing", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::CHECK_BACKUP_TYPE);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    auto pluginManJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    auto action = pluginManJob->MakeAction([](){return StubSuccess();}, PluginMainJob::ActionEvent::GENE_POST_JOB);
    EXPECT_NE(action, nullptr);
    initAction->OnTransition = action;
    g_call = false;
    auto result = initAction->Transition();
    EXPECT_EQ(g_call, true);
}

/*
 * 用例名称：ExcutorBuild执行Next构建成功
 * 前置条件：1. ExcutorBuild执行Next
 * check点：检查Build是否产生执行器
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_Next_Call_Build_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.Next(
        [&result](int32_t) {
            result+= "Executor1 ";
            return 0;
        }).Next(
         [&result](int32_t) {
            result+= "Executor2 ";
            return 0;});
    EXPECT_EQ(builder.m_executors.size(), 2);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor2 ");
}

/*
 * 用例名称：ExcutorBuild执行ConditonNext构建成功
 * 前置条件：1. ExcutorBuild执行ConditonNext
 * check点：检查ConditonNext成功分支执行
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_ConditonNext_Call_Build_Success_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.ConditonNext(
        [&result](int32_t) {
            result+= "Executor1 ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Failed";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Success";
            return 0;
        });
    EXPECT_EQ(builder.m_executors.size(), 1);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor2 Success");
}

/*
 * 用例名称：ExcutorBuild执行ConditonNext构建成功
 * 前置条件：1. ExcutorBuild执行ConditonNext
 * check点：检查ConditonNext失败分支执行
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_ConditonNext_Call_Build_Failed_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.ConditonNext(
        [&result](int32_t) {
            result+= "Executor1 ";
            return -1;
        },
        [&result](int32_t) {
            result+= "Executor2 Failed";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Success";
            return 0;
        });
    EXPECT_EQ(builder.m_executors.size(), 1);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor2 Failed");
}

/*
 * 用例名称：ExcutorBuild执行ConditonNext和Next构建成功
 * 前置条件：1. ExcutorBuild执行ConditonNext
 * check点：检查ConditonNext和Next串行执行
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_ConditonNext_Call_Build_Failed_And_Next_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.ConditonNext(
        [&result](int32_t) {
            result+= "Executor1 ";
            return -1;
        },
        [&result](int32_t) {
            result+= "Executor2 Failed ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Success ";
            return 0;
        }).Next(
        [&result](int32_t) {
            result+= "Executor3 Finally";
            return 0;
        });
    EXPECT_EQ(builder.m_executors.size(), 2);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor2 Failed Executor3 Finally");
}

/*
 * 用例名称：ExcutorBuild执行ConditonNext和Next构建成功
 * 前置条件：1. ExcutorBuild执行ConditonNext
 * check点：检查ConditonNext和Next串行执行
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_ConditonNext_Call_Build_Success_And_Next_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.ConditonNext(
        [&result](int32_t) {
            result+= "Executor1 ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Failed ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Success ";
            return 0;
        }).Next(
        [&result](int32_t) {
            result+= "Executor3 Finally";
            return 0;
        });
    EXPECT_EQ(builder.m_executors.size(), 2);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor2 Success Executor3 Finally");
}

/*
 * 用例名称：ExcutorBuild执行ConditonNext和Next构建成功
 * 前置条件：1. ExcutorBuild执行ConditonNext
 * check点：检查ConditonNext重试分支
 */
TEST_F(PluginMainJobTest, ExecutorBuilder_ConditonNext_Call_Build_REDO_Failed_And_Next_test_ok)
{
    DoLogTest();

    ExecutorBuilder builder;
    std::string result;
    builder.ConditonNext(
        [&result](int32_t) {
            result+= "Executor1 ";
            return MP_REDO;
        },
        [&result](int32_t) {
            result+= "Executor2 Failed ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor2 Success ";
            return 0;
        },
        [&result](int32_t) {
            result+= "Executor3 redo ";
            return 0;
        }).Next(
        [&result](int32_t) {
            result+= "Executor4 Finally";
            return 0;
        });
    EXPECT_EQ(builder.m_executors.size(), 2);
    auto executor = builder.Build();
    EXPECT_EQ(builder.m_executors.size(), 0);
    int32_t ret = 0;
    executor(ret);
    EXPECT_EQ(result, "Executor1 Executor3 redo Executor4 Finally");
}

/*
 * 用例名称：PluginJobFactory构建子任务
 * 前置条件：1. 获取到PluginJobFactory和任务Data
 * check点：构建的子任务类型是否正确
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Create_Job_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    std::shared_ptr<PluginSubPrepJob> preJob = std::dynamic_pointer_cast<PluginSubPrepJob>(job);
    EXPECT_NE(preJob, nullptr);

    jobData = {"pluginName", "mainJobID", "mainJobID_Post", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    std::shared_ptr<PluginSubPostJob> postJob = std::dynamic_pointer_cast<PluginSubPostJob>(job);
    EXPECT_NE(postJob, nullptr);

    jobData = {"pluginName", "mainJobID", "mainJobID_Busi", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    std::shared_ptr<PluginSubBusiJob> busiJob = std::dynamic_pointer_cast<PluginSubBusiJob>(job);
    EXPECT_NE(busiJob, nullptr);

    jobData = {"pluginName", "mainJobID", "mainJobID_Gen", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::GENERATE_SUB_JOB};
    job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    std::shared_ptr<PluginSubGeneJob> geneJob = std::dynamic_pointer_cast<PluginSubGeneJob>(job);
    EXPECT_NE(geneJob, nullptr);
}

/*
 * 用例名称：构建PluginSubPrepJob任务执行流程
 * 前置条件：1. 构建PluginSubPrepJob任务成功
 * check点：执行PluginSubPrepJob任务检查插件执行成功后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Pre_Job_SUCCESS_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClientAndPluginReturnFailed);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(excutorResult, "AsyncBackupPrerequisite Failed ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginSubPrepJob任务执行流程
 * 前置条件：1. 构建PluginSubPrepJob任务成功
 * check点：执行PluginSubPrepJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Pre_Job_FAILED_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifyFailedStub);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    g_waitFailedCall = false;
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(g_waitFailedCall, true);
    EXPECT_EQ(excutorResult, "AsyncBackupPrerequisite Success WaitPluginNotify Failed ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Pre_Job_SUCCESS_And_Wait_Success_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    g_waitSuccessCall = false;
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(g_waitSuccessCall, true);
    EXPECT_EQ(excutorResult, "AsyncBackupPrerequisite Success WaitPluginNotify Success ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::Running));
}

/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Busi_Job_MOUNT_FAILED_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasFailedStub);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(excutorResult, "MountNas Failed ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Busi_Job_Call_Plugin_FAILED_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClientAndPluginReturnFailed);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);
    stub.set(&Job::UmountNas, UnmountNasSuccessStub);
    

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(excutorResult, "MountNas Success AsyncExecuteBackupSubJob Failed ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Busi_Job_Call_Plugin_BUT_PLUGIN_REPORT_FAILED_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);
    stub.set(&Job::UmountNas, UnmountNasSuccessStub);
    

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(excutorResult, "MountNas Success AsyncExecuteBackupSubJob Success ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::Running));
}

/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Post_Job_SUCCESS_And_Wait_Failed_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifyFailedStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubJobConvertStringtoJsonSuccess);
    stub.set(&PluginSubPostJob::GetJobsExecResult, StubGetJobsExecResult);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    excutorResult = "";
    job->Exec();
    EXPECT_EQ(excutorResult, "MountNas Success AsyncBackupPostJob Success ");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::Running));
}

/*
 * 用例名称：构建PluginSubPostJob任务获取任务状态
 * 前置条件：无
 * check点：执行PluginSubBusiJob任务获取状态是否成功
 */

TEST_F(PluginMainJobTest, PluginSubPostJobStatusTest)
{
    DoLogTest();
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_post", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    auto pjob = std::dynamic_pointer_cast<PluginSubPostJob>(job);
    EXPECT_NE(pjob, nullptr);
    std::string str = "{\"mainTask\":1,\"subTask\":{\"total\":3,\"success\":1,\"failed\":0, \"aborted\": 1}}";
    Json::Value rspValue;
    CJsonUtils::ConvertStringtoJson(str, rspValue);
    auto ret = pjob->GetJobStatus(rspValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(pjob->m_wholeJobResult, JobResult::type::ABORTED);
    str = "{\"mainTask\":1,\"subTask\":{\"total\":3,\"success\":1,\"failed\":0, \"aborted\": 0}}";
    CJsonUtils::ConvertStringtoJson(str, rspValue);
    ret = pjob->GetJobStatus(rspValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(pjob->m_wholeJobResult, JobResult::type::SUCCESS);
    str = "{\"mainTask\":2,\"subTask\":{\"total\":1,\"success\":1,\"failed\":0, \"aborted\": 0}}";
    CJsonUtils::ConvertStringtoJson(str, rspValue);
    ret = pjob->GetJobStatus(rspValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(pjob->m_wholeJobResult, JobResult::type::ABORTED);
    str = "{\"mainTask\":7,\"subTask\":{\"total\":1,\"success\":1,\"failed\":0, \"aborted\": 0}}";
    CJsonUtils::ConvertStringtoJson(str, rspValue);
    ret = pjob->GetJobStatus(rspValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(pjob->m_wholeJobResult, JobResult::type::FAILED);
    str = "{\"mainTask\":1,\"subTask\":{\"total\":1,\"success\":1,\"failed\":0, \"aborted\": 0}}";
    CJsonUtils::ConvertStringtoJson(str, rspValue);
    ret = pjob->GetJobStatus(rspValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(pjob->m_wholeJobResult, JobResult::type::SUCCESS);
}


/*
 * 用例名称：构建PluginSubBusiJob任务执行流程
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob任务检查插件执行失败后执行流程是否ok
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Main_job_Mount_nas_failed_report_label_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient); 
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasFailedStub);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
 
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    job->Initialize();
    excutorResult = "";
    job->Exec();
    EXPECT_EQ("MountNas Failed ", excutorResult);
    EXPECT_EQ(static_cast<int32_t>(AppProtect::MainJobState::FAILED), job->GetData().status);
}

/*
 * 用例名称：执行前置任务设置qos策略
 * 前置条件：MountNas CheckBackupJobType执行成功
 * check点：qos设置成功，检查设置后值，接口调用成功
 */
TEST_F(PluginMainJobTest, ExecutePreSubJob_SetQos_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::GenerateMainJob, StubSuccess);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubSuccess);

    mp_string strValue = R"({"taskId" : "","appInfo" :{"type" : "","subType" : "",},
    "taskParams" :{"backupType" : "","copyFormat" : 0, "dataLayout":{},"filters" : [],"scripts":{},
    "qos" : {"bandwidth": 1000,"iops": null}}, "taskType" : 1,"repositories" :
    [{"path" : ["/mnt/databackup/test"],"type" : 1},{"path" : ["/mnt/databackup/test"],"type" : 2}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    PluginJobData jobData = {"pluginName", "mainJobID", "", jsValue,  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    pJob.get()->Initialize();
    pJob.get()->Exec();
    EXPECT_EQ(1000, pJob->GetData().param["taskParams"]["qos"]["bandwidth"].asInt());
}


/*
 * 用例名称：构建PluginMainJob任务执行流程
 * 前置条件：1. 构建PluginMainJob任务成功
 * check点：执行PluginMainJob任务失败后是否上报任务进度
 */
TEST_F(PluginMainJobTest, PluginMainJob_After_Exec_Failed_send_to_dme_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClientAndPluginReturnFailed);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&ReportJobDetailFactory::SendDetailToDme, StubSendDetailToDme);
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&Job::MountNas, StubJobExecFailed);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    g_stage = 0;
    g_call = false;
    job->Initialize();
    job->Exec();
    EXPECT_EQ(g_call, true);
    EXPECT_EQ(g_stage, mp_uint32(MainJobState::FAILED));
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(MainJobState::FAILED));
}

/*
 * 用例名称：构建PluginMainJob任务执行流程
 * 前置条件：1. 构建PluginMainJob任务成功
 * check点：执行PluginMainJob任务成功后是否上报任务进度
 */
TEST_F(PluginMainJobTest, PluginMainJob_After_Exec_Success_send_to_dme_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&ReportJobDetailFactory::SendDetailToDme, StubSendDetailToDme);
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&Job::MountNas, StubJobExecSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubJobExecSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    g_stage = 0;
    g_call = false;
    job->Initialize();
    job->Exec();
    EXPECT_EQ(g_call, true);
    EXPECT_EQ(g_stage, mp_uint32(MainJobState::COMPLETE));
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(MainJobState::COMPLETE));
}

/*
 * 用例名称：构建PluginPostJob任务执行流程上报子任务状态
 * 前置条件：1. 构建PluginPostJob任务成功
 * check点：执行PluginPostJob任务失败后是否上报label
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Post_Job_Exec_FAILED_send_dme_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClientAndPluginReturnFailed);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);
    stub.set(&Job::UmountNas, UnmountNasSuccessStub);
    stub.set(&ReportJobDetailFactory::SendDetailToDme, StubSendDetailToDme);
    stub.set(&PluginSubPostJob::GetJobsExecResult, StubJobExecSuccess);
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    g_label = "";
    job->Exec();
    EXPECT_EQ(g_label, "agent_execute_post_task_fail_label");
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginPostJob任务执行流程上报子任务状态
 * 前置条件：1. 构建PluginPostJob任务成功
 * check点：执行PluginPostJob任务成功后是否上报label
 */
TEST_F(PluginMainJobTest, PluginJobFactory_Check_Post_Job_Exec_Success_send_dme_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, WaitPluginNotifySuccessStub);
    stub.set(&Job::MountNas, MountNasSuccessStub);
    stub.set(&Job::UmountNas, UnmountNasSuccessStub);
    stub.set(&ReportJobDetailFactory::SendDetailToDme, StubSendDetailToDme);
    stub.set(&PluginSubPostJob::GetJobsExecResult, StubJobExecSuccess);
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Pre", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);

    g_label = "";
    job->Exec();
    EXPECT_EQ(g_label, "agent_execute_post_script_success_label");// agent_execute_post_script_success_label 在 agent_start_execute_post_task_success_label 之后
    EXPECT_EQ(job->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::Running));
}


/*
 * 用例名称：构建PluginPostJob任务执行流程上报子任务状态
 * 前置条件：1. 构建PluginPostJob任务成功
 * check点：执行PluginPostJob任务失败后是否上报label
 */
TEST_F(PluginMainJobTest, Busi_Subjob_pausejob_retry_failed_test_ok)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);

    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_busijob", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    busiJob->UpdateStatus(mp_uint32(AppProtect::SubJobState::Running));
    busiJob->NotifyPauseJob();    
    EXPECT_EQ(busiJob->NeedRetry(), true);
    EXPECT_EQ(busiJob->GetData().status, static_cast<int32_t>(AppProtect::SubJobState::SubJobFailed));
}

/*
 * 用例名称：构建PluginSubBusiJob任务是否可在当前节点运行检查
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob是否可在当前节点运行检查
 */
TEST_F(PluginMainJobTest, Busi_Subjob_can_run_in_local_node_ok)
{
    DoLogTest();
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(PluginSubBusiJob, ExecAllowBackupInLocalNode), StubExecAllowBackupInLocalNode);

    Json::Value extendInfo;
    extendInfo["role"] = "1";
    Json::Value node;
    node["uuid"] = "123456";
    node["extendInfo"] = extendInfo;
    Json::Value nodes;
    nodes.append(node);
    Json::Value envInfo;
    envInfo["nodes"] = nodes;
    Json::Value param;
    param["envInfo"] = envInfo;
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_busijob", param,  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData); 
    EXPECT_EQ(busiJob->CanbeRunInLocalNode(), MP_SUCCESS);
}

/*
 * 用例名称：构建PluginSubBusiJob任务是否可在当前节点运行检查
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob是否可在当前节点运行检查
 */
TEST_F(PluginMainJobTest, Busi_Subjob_can_run_in_local_node_no_role_ok)
{
    DoLogTest();
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(PluginSubBusiJob, ExecAllowBackupInLocalNode), StubExecAllowBackupInLocalNode);

    Json::Value envInfo;
    Json::Value param;
    param["envInfo"] = envInfo;
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_busijob", param,  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData); 
    EXPECT_EQ(busiJob->CanbeRunInLocalNode(), MP_SUCCESS);
}

/*
 * 用例名称：构建PluginSubBusiJob任务在优先备策略下是否可在当前节点运行检查
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob在优先备策略下是否可在当前节点运行检查
 */
TEST_F(PluginMainJobTest, Busi_Subjob_can_run_in_local_node_policy_ok)
{
    DoLogTest();
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(PluginSubBusiJob, ExecAllowBackupInLocalNode), StubExecAllowBackupInLocalNode);

    Json::Value extendInfo;
    extendInfo["role"] = "1";
    Json::Value node;
    node["uuid"] = "123456";
    node["extendInfo"] = extendInfo;
    Json::Value nodes;
    nodes.append(node);
    Json::Value envInfo;
    envInfo["nodes"] = nodes;
    Json::Value ext;
    ext["slave_node_first"] = "true";
    Json::Value param;
    param["envInfo"] = envInfo;
    param["extendInfo"] = ext;
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_busijob", param,  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData); 
    EXPECT_EQ(busiJob->CanbeRunInLocalNode(), MP_SUCCESS);
}

/*
 * 用例名称：生成后置任务
 * 前置条件：1.Mock log 2、Mock生成后置任务之外的其他步骤
 * check点：1、未下发多后置任务标志，生成后置任务成功
            2、下发多后置任务标志，生成后置任务成功
 */
TEST_F(PluginMainJobTest, ExecGenerateMainJob_Success)
{
    DoLogTest();
    stub.set(&Job::SplitRepositories, StubSuccess);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&JobStateDB::QueryJob, StubExistJob);
    stub.set(&JobStateDB::UpdateStatus, StubSuccess);
    stub.set(&PluginMainJob::CheckBackupJobType, StubSuccess);
    stub.set(&PluginMainJob::SetQosStrategy, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreScript, StubSuccess);
    stub.set(&PluginMainJob::ExecutePreSubJob, StubSuccess);
    stub.set(&PluginMainJob::ExecGenerateSubJob, StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());

    Json::Value jobParam;
    jobParam["extendInfo"]["multiPostJob"] = "true";
    jobData = {"pluginName", "mainJobID", "", jobParam,  AppProtect::MainJobType::BACKUP_JOB};
    pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    EXPECT_EQ(MP_SUCCESS, pJob.get()->Initialize());
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
 * 用例名称：构建PluginSubBusiJob任务在优先备策略下是否可在当前节点运行检查
 * 前置条件：1. 构建PluginSubBusiJob任务成功
 * check点：执行PluginSubBusiJob在优先备策略下是否可在当前节点运行检查
 */
TEST_F(PluginMainJobTest, PostJob_CanbeRunInLocalNode)
{
    DoLogTest();
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);

    {
        PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_postjob", Json::Value(),
            AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
        auto pJob = std::make_shared<PluginSubPostJob>(jobData); 
        EXPECT_EQ(MP_SUCCESS, pJob->CanbeRunInLocalNode());
    }
    {
        PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_postjob", Json::Value(),
            AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::POST_SUB_JOB};
        auto pJob = std::make_shared<PluginSubPostJob>(jobData); 
        EXPECT_EQ(MP_SUCCESS, pJob->CanbeRunInLocalNode());
    }
}

PluginJobData GetJobData(){
    PluginJobData jobData;
    jobData.appType = "appType";
    jobData.mainID = "mainid";
    jobData.subID= "subid";
    Json::Value param, failedAgenta, failedAgentb, agenta, agentb;
    failedAgenta["id"] = "failedAgent1";
    failedAgentb["id"] = "failedAgent2";
    agenta["id"] = "agent1";
    agentb["id"] = "agent2";
    param["failedAgents"].append(failedAgenta);
    param["failedAgents"].append(failedAgentb);
    param["agents"].append(agenta);
    param["agents"].append(agentb);
    jobData.param = param;
    return jobData;
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_NotifyPluginReloadImplT)
{
    PluginJobData jobData = {"pluginName", "mainJobID", "mainJobID_Busi", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    std::shared_ptr<PluginSubBusiJob> busiJob = std::dynamic_pointer_cast<PluginSubBusiJob>(job);
    mp_string appType = "type";
    mp_string newPluginPID = "id";
    bool ret = busiJob->NotifyPluginReloadImpl(appType, newPluginPID);
    EXPECT_EQ(ret, true);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_CheckAgentFailedT)
{
    PluginJobData jobData = GetJobData();
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    mp_string nodeId = "failedAgent2";
    bool ret = busiJob->CheckAgentFailed(nodeId);
    EXPECT_EQ(ret, true);
    nodeId = "fdfdgf";
    ret = busiJob->CheckAgentFailed(nodeId);
    EXPECT_EQ(ret, false);
    mp_string nullstr;
    ret = busiJob->CheckAgentFailed(nullstr);
    EXPECT_EQ(ret, true);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_CheckIfCanRunT)
{
    Json::Value roleInfo, rolea, roleb, extendInfo;
    rolea["uuid"] = "roleauuida";
    roleb["uuid"] = "roleauuidb";
    extendInfo["role"] = "1";
    rolea["extendInfo"] = extendInfo;
    roleb["extendInfo"] = extendInfo;
    roleInfo.append(rolea);
    roleInfo.append(roleb);
    mp_string nodeId = "roleauuida";
    mp_int32 role;
    PluginJobData jobData = GetJobData();
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    busiJob->GetRole(nodeId, roleInfo, role);
    EXPECT_EQ(role, 1);
    BackupLimit::type policy = BackupLimit::FIRST_MASTER;
    mp_int32 ret = busiJob->CheckIfCanRun(roleInfo, policy);
    EXPECT_EQ(ret, 0);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_ParseRoleInfoT)
{
    Json::Value roleInfo, envInfo, rolea, role;
    rolea["uuid"] = "roleauuida";
    role.append(rolea);
    envInfo["nodes"] = role;
    PluginJobData jobData = GetJobData();
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    bool ret = busiJob->ParseRoleInfo(roleInfo);
    EXPECT_EQ(ret, false);
    jobData.param["envInfo"] = envInfo;
    auto busiJobT = std::make_shared<PluginSubBusiJob>(jobData);
    ret = busiJobT->ParseRoleInfo(roleInfo);
    EXPECT_EQ(ret, true);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_ParsePolicyT)
{
    Json::Value extendInfo;
    extendInfo["slave_node_first"] = "true";
    BackupLimit::type policy;
    PluginJobData jobData = GetJobData();
    jobData.param["extendInfo"] = extendInfo;
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    busiJob->ParsePolicy(policy);
    EXPECT_EQ(policy, BackupLimit::FIRST_SLAVE);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_NeedExecPolicyT)
{
    PluginJobData jobData = GetJobData();
    jobData.param["subTaskParams"] = "{\"policy\":0}";
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    bool ret = busiJob->NeedExecPolicy();
    EXPECT_EQ(ret, true);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_NeedExecPolicyTb)
{
    PluginJobData jobData = GetJobData();
    jobData.param["subTaskParams"] = "djlkafjkdj";
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    bool ret = busiJob->NeedExecPolicy();
    EXPECT_EQ(ret, false);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_CanbeRunInLocalNodeForRestoreT)
{
    PluginJobData jobData = GetJobData();
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    mp_int32 ret = busiJob->CanbeRunInLocalNodeForRestore();
    EXPECT_EQ(ret, 1677929228);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_CanbeRunInLocalNodeForCheckCopyT)
{
    PluginJobData jobData = GetJobData();
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    mp_int32 ret = busiJob->CanbeRunInLocalNodeForCheckCopy();
    EXPECT_EQ(ret, 1677929228);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_CanbeRunInLocalNodeForBackupT)
{
    PluginJobData jobData = GetJobData();
    jobData.param["subTaskParams"] = "{\"policy\":0}";
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    mp_int32 ret = busiJob->CanbeRunInLocalNodeForBackup();
    EXPECT_EQ(ret, 1677929228);
}

TEST_F(PluginMainJobTest, PluginSubBusiJob_ExecAllowBackupInLocalNodeT)
{
    PluginJobData jobData = GetJobData();
    jobData.param["subTaskParams"] = "{\"policy\":0}";
    auto busiJob = std::make_shared<PluginSubBusiJob>(jobData);
    mp_int32 ret = busiJob->ExecAllowBackupInLocalNode();
    EXPECT_EQ(ret, 1677929228);
}

