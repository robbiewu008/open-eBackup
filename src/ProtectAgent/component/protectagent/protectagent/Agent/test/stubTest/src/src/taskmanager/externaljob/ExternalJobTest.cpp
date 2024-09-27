/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  job test
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */

#include "taskmanager/externaljob/ExternalJobTest.h"
#include "taskmanager/externaljob/JobPool.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "thriftservice/include/IThriftService.h"
#include "servicecenter/timerservice/detail/TimerService.h"
#include "taskmanager/externaljob/PluginSubPostJob.h"
#include "taskmanager/externaljob/PluginSubBusiJob.h"
#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "taskmanager/externaljob/PluginSubGeneJob.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "message/curlclient/DmeRestClient.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "securecom/RootCaller.h"
#include "taskmanager/externaljob/Job.h"
#include "host/host.h"

using namespace servicecenter;
using namespace thriftservice;

static std::shared_ptr<ProtectServiceIf> g_ProtectServiceClient;
static const int G_LOCAL_PORT = 9090;

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

std::shared_ptr<thriftservice::IThriftClient> StubGetThriftClient()
{
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "127.0.0.1", 59610 };
    std::shared_ptr<thriftservice::IThriftClient> thriftclient = thriftservice->RegisterClient(opt);
    return thriftclient;
}

std::shared_ptr<ProtectServiceIf> StubGetProtectServiceClient(std::shared_ptr<thriftservice::IThriftClient> pThriftClient)
{
    if (g_ProtectServiceClient.get() == nullptr) {
        g_ProtectServiceClient = std::make_shared<StubProtectServiceClient>(nullptr);
    }
    return g_ProtectServiceClient;
}

std::shared_ptr<ProtectServiceIf> StubGetThriftClientNullptr()
{
    return nullptr;
}

mp_int32 StubSuccess(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pThis)
{
    return MP_FAILED;
}

void StubVoid(mp_void* pThis)
{

}

mp_int32 StubMountNasFileSystem(void* obj, MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, bool multiFileSystem, const MountPermission &permit)
{
    successMountPath.push_back("/mnt/databackup/pluginName/mainJobID/data/Share1/192.168.145.161");
    return MP_SUCCESS;
}

static bool g_call = false;
mp_int32 StubRestSendResponseTestSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value rspValue;
    rspValue["errorCode"] = "0";
    httpResponse.body = rspValue.toStyledString();
    g_call = true;
    return MP_SUCCESS;
}

mp_int32 ReportJobDetailsStub(AppProtect::ActionResult& _return, const AppProtect::SubJobDetails& jobInfo)
{
    return MP_SUCCESS;
}

mp_int32 ExecScript(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckIsDoradoEnvironment(mp_void* pThis, mp_bool& isDorado)
{
    isDorado = true;
    return MP_SUCCESS;
}

mp_int32 StubSendRequestSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    return MP_SUCCESS;
}

mp_int32 StubGetMountNasParam(MountNasParam& param, const PluginJobData &data, const StorageRepository &stRep,
    const MountPermission &permit)
{
    return MP_SUCCESS;
}

void StubGetJobPermission(mp_void* pThis, AppProtect::JobPermission &jobPermit)
{
    Json::Value jsonValue;
    jsonValue["extendInfo"]["path"] = "/tmp";
    jobPermit.__set_extendInfo(jsonValue["extendInfo"].toStyledString());
}

mp_int32 StubGetCancelLivemountJobName(mp_void* pThis, std::vector<mp_string> &vecJobName)
{
    vecJobName.push_back("umount");
    return MP_SUCCESS;
}

/*
* 用例名称：创建任务池，放入任务，销毁任务池
* 前置条件：1、正常创建任务池
* check点：1、正常放入任务  2、正常销毁任务池 3、获取到已执行任务指针不为空
*/
TEST_F(ExternalJobTest, JobPool)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);  
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    JobPool pool;
    pool.CreatePool(10);

    {
        PluginJobData jobData = {"pluginName", "main1", "sub1", Json::Value(), 
            AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
        std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        pool.PushJob(pJob);
    }
    {
        PluginJobData jobData = {"pluginName", "main2", "sub2", Json::Value(), 
            AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
        std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        pool.PushJob(pJob);
    }
    pool.DestoryPool();
}

/*
* 用例名称：任务状态是否为失败
* 前置条件：
* check点：1、主任务不是失败  2、子任务不是失败
*/
TEST_F(ExternalJobTest, IsFailed_TEST)
{
    {
        PluginJobData jobData = {"pluginName", "main1", "", Json::Value(), 
            AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
        std::shared_ptr<Job> pMainJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        EXPECT_EQ(false, pMainJob->IsFailed());
    }
    {
        PluginJobData jobData = {"pluginName", "main1", "sub1", Json::Value(), 
            AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
        std::shared_ptr<Job> pSubJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        EXPECT_EQ(false, pSubJob->IsFailed());
    }
}

/*
* 用例名称：执行备份前置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的前置备份任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPreJob_Backup)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::PRE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行恢复前置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的前置恢复任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPreJob_Restore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::PRE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient); 
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行即时恢复前置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的前置即时恢复任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPreJob_Inrestore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::INSTANT_RESTORE_JOB, SubJobType::type::PRE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubPrepJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行备份分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的备份分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_Backup)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行恢复分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的恢复分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_Restore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行挂载分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的挂载分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_Livemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行挂载分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的挂载分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_CancelLivemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::CANCEL_LIVEMOUNT_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行即时恢复分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的即时恢复分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_Inrestore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::INSTANT_RESTORE_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行索引分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的索引分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_BuildIndex)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BUILD_INDEX_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行删除分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的删除分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_DelCopy)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::DELETE_COPY_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行备份任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的备份任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_Backup)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行恢复任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的恢复任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_Restore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行挂载任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的挂载任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_Livemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行挂载任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的挂载任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_CancelLivemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::CANCEL_LIVEMOUNT_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行即时恢复任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的即时恢复任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_Inrestore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::INSTANT_RESTORE_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行索引任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的索引任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_BuildIndex)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(),
        AppProtect::MainJobType::BUILD_INDEX_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行删除任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的索引任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_DelCopy)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(),
        AppProtect::MainJobType::DELETE_COPY_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行备份后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的备份后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_Backup)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(PluginSubPostJob, GetJobsExecResult), StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行恢复后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的恢复后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_Restore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(PluginSubPostJob, GetJobsExecResult), StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行索引后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的索引后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_BuildIndex)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BUILD_INDEX_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    typedef bool (*pRestClient)(AppProtect::AppProtectJobHandler*, AppProtect::ActionResult&, const AppProtect::SubJobDetails&);
    stub.set((pRestClient)(&AppProtect::AppProtectJobHandler::ReportJobDetails), ReportJobDetailsStub);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行挂载后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的挂载后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_Livemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    typedef bool (*pRestClient)(AppProtect::AppProtectJobHandler*, AppProtect::ActionResult&, const AppProtect::SubJobDetails&);
    stub.set((pRestClient)(&AppProtect::AppProtectJobHandler::ReportJobDetails), ReportJobDetailsStub);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行取消挂载后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的取消挂载后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_CancelLivemount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::CANCEL_LIVEMOUNT_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    typedef bool (*pRestClient)(AppProtect::AppProtectJobHandler*, AppProtect::ActionResult&, const AppProtect::SubJobDetails&);
    stub.set((pRestClient)(&AppProtect::AppProtectJobHandler::ReportJobDetails), ReportJobDetailsStub);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行即时恢复后置任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的即时恢复后置任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubPostJob_Inrestore)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::INSTANT_RESTORE_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(PluginSubPostJob, GetJobsExecResult), StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}
/*
* 用例名称：中止子任务成功，子任务还未发送到插件
* 前置条件：1、agent框架中已有子任务 2. 任务还未发送到插件
* check点：中止子任务成功
*/
TEST_F(ExternalJobTest, AbortSubjobSuccessNotSendPlugin)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetProtectServiceClient), StubGetThriftClientNullptr);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    pJob->Abort();
    EXPECT_EQ((mp_uint32)SubJobState::SubJobComplete, pJob->GetData().status);
}
/*
* 用例名称：中止子任务下发成功，子任务已经发送到插件
* 前置条件：1、agent框架中已有子任务 2. 任务已经发送到插件 3. 插件当前状态为运行中
* check点：中止子任务下发成功
*/
TEST_F(ExternalJobTest, AbortSubjobSendPluginSuccess)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, SendAbortToPlugin), StubSuccess);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(ADDR(PluginSubJob, RemovePluginTimer), StubVoid);
    pJob->Exec();
    EXPECT_EQ((mp_uint32)SubJobState::Running, pJob->GetData().status);
}
/*
* 用例名称：不能中止后置子任务
* 前置条件：1、后置子任务在agent任务池中存在
* check点：不能中止后置子任务
*/
TEST_F(ExternalJobTest, AbortPostJobFailed)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    mp_int32 ret = pJob->Abort();
    EXPECT_EQ(MP_FAILED, ret);
}
/*
* 用例名称：子任务在正常从运行状态切换至完成状态
* 前置条件：1、子任务正在运行 2. 插件定期上报状态
* check点：子任务状态切换正常
*/
TEST_F(ExternalJobTest, SubjobStateChangeNormal)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient); 
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    servicecenter::ServiceFactory::GetInstance()->Register<timerservice::detail::TimerService>("ITimerService"); 
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());

    AppProtect::SubJobDetails detail;
    detail.jobId = "mainJobID";
    detail.subJobId = "subJobID";
    detail.jobStatus = SubJobStatus::type::COMPLETED;
    pJob->NotifyJobDetail(detail);
    EXPECT_EQ((mp_uint32)SubJobState::SubJobComplete, pJob->GetData().status);
}

/*
* 用例名称：子任务在正常从运行状态切换至完成状态后，清理掉定时器资源
* 前置条件：1、子任务正在运行 2. 插件定期上报状态
* check点：资源被清理
*/
TEST_F(ExternalJobTest, SubjobStateChangeResrouceCleanSuccess)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    std::shared_ptr<PluginSubJob> subjob = std::dynamic_pointer_cast<PluginSubJob>(pJob);
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient); 
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    servicecenter::ServiceFactory::GetInstance()->Register<timerservice::detail::TimerService>("ITimerService"); 

    /*======== 状态从UNDEFINE切换到PrepareFailed ============*/
    subjob->m_data.status = 255;
    subjob->ChangeState(SubJobState::UNDEFINE);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::PrepareFailed);
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);

    /*======== 状态从Running切换到SubJobComplete ============*/
    subjob->ChangeState(SubJobState::Running);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::SubJobComplete);
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);

    /*======== 状态从Running切换到SubJobFailed ============*/
    subjob->ChangeState(SubJobState::Running);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::SubJobFailed );
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);
}


/*
* 用例名称：任务超时终止接口下发插件执行
* 前置条件：1、任务下发PauseJob接口调用
* check点：接口返回成功
*/
TEST_F(ExternalJobTest, Job_Pause_Job_call_thrift_interface_test_ok)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    AppProtect::SubJobDetails detail;
    auto ret = pJob->PauseJob();
    EXPECT_EQ(ret, MP_SUCCESS);
}

/*
* 用例名称：删除qos策略
* 前置条件：
* check点：接口调用成功
*/
TEST_F(ExternalJobTest, DeleteQosStrategy_stub)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    mp_string strValue = R"({"taskId" : "","appInfo" :{"type" : "","subType" : "",},
    "taskParams" :{"backupType" : "","copyFormat" : 0, "dataLayout":{},"filters" : [],"scripts":{},
    "qos" : {"bandwidth": 1000,"iops": null}}, "taskType" : 1,"repositories" :
    [{"path" : ["/mnt/databackup/test"],"type" : 3},{"path" : ["/mnt/databackup/test"],"type" : 2}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", jsValue, 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(PluginSubPostJob, GetJobsExecResult), StubSuccess);
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponseTestSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：是否手动挂载
* 前置条件：
* check点：接口调用成功
*/
TEST_F(ExternalJobTest, Is_ManualMount)
{
    mp_string strValue = R"({"extendInfo": {"manualMount": "true"}})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    PluginJobData jobData;
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_EQ(MP_TRUE, pJob->IsManualMount(jsValue));
    strValue = R"({"extendInfo": {"manualMount": "false"}})";
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    EXPECT_EQ(MP_FALSE, pJob->IsManualMount(jsValue));
}

/*
* 用例名称：非原生格式恢复挂载文件系统
* 前置条件：恢复任务
* check点：1、任务执行成功
           2、下发多副本挂载路径
*/
TEST_F(ExternalJobTest, non_native_restore_job_test)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(&Repository::GetMountNasParam, StubGetMountNasParam);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    mp_string strValue = R"({"copies": [{"format": 1,"repositories": [{"remotePath": "/Database/source","type": 0, 
    "remoteHost": [{"ip": "192.168.1.1"}]}, {"remotePath": "/Database","type": 1, "remoteHost": [{"ip": "192.168.1.1"}]}, 
    {"remotePath": "/Database/f3cec253","type": 2, "remoteHost": [{"ip": "192.168.1.1"}]}],"type": "full"}, {"format": 1,
    "repositories": [{"remotePath": "/clone_0/source","type": 0, "remoteHost": [{"ip": "192.168.1.1"}]}, 
    {"remotePath": "/clone_0","type": 1}],"type": "increment"}],"repositories": [{"remotePath": "","type": 1}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", jsValue, 
        AppProtect::MainJobType::RESTORE_JOB, SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(PluginSubPostJob, GetJobsExecResult), StubSuccess);
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponseTestSuccess);

    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    EXPECT_EQ(0, pJob->GetData().param["copies"][0]["repositories"][0]["path"].size());
    EXPECT_EQ(1, pJob->GetData().param["copies"][1]["repositories"][0]["path"].size());
}

/*
* 用例名称：执行副本校验分解任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的删除分解任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubGeneJob_CheckCopy)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(), 
        AppProtect::MainJobType::CHECK_COPY_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
    stub.set(&PluginSubGeneJob::WaitPluginNotify, StubFailed);
    EXPECT_EQ(MP_FAILED, pJob->Exec());
}

/*
* 用例名称：执行副本校验任务
* 前置条件：1、通过ExternalPluinManager获取thrift接口指针
* check点：1、创建的索引任务指针不为空 2 正常执行任务
*/
TEST_F(ExternalJobTest, SubBusiJob_CheckCopy)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Job::MountNas, StubSuccess);
    stub.set(&PrepareFileSystem::MountNasFileSystem, StubMountNasFileSystem);
    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);

    PluginJobData jobData = {"pluginName", "mainJobID", "subJobID", Json::Value(),
        AppProtect::MainJobType::CHECK_COPY_JOB, SubJobType::type::BUSINESS_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(nullptr, pJob.get());

    stub.set(ADDR(Job, GetThriftClient), StubGetThriftClient);  
    stub.set(ADDR(Job, GetProtectServiceClient), StubGetProtectServiceClient);
    EXPECT_EQ(MP_SUCCESS, pJob->Exec());
}

/*
* 用例名称：执行脚本命令注入拦截
* 前置条件：
* check点：1、脚本存在命令注入, 脚本拦截成功 2.正常脚本不做拦截
*/
TEST_F(ExternalJobTest, ScriptCommandInject)
{
    mp_string dangerousScript = "/opt/`sleep$IFS$999`.sh";
    mp_string normalScript = "/opt/pre.sh";
    PluginJobData jobData;
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_EQ(MP_FALSE, pJob->IsScriptValid(dangerousScript));
    EXPECT_EQ(MP_TRUE, pJob->IsScriptValid(normalScript));
}