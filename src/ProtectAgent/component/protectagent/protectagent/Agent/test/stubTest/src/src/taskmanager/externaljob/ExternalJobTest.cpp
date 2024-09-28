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
* �������ƣ���������أ������������������
* ǰ��������1���������������
* check�㣺1��������������  2��������������� 3����ȡ����ִ������ָ�벻Ϊ��
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
* �������ƣ�����״̬�Ƿ�Ϊʧ��
* ǰ��������
* check�㣺1����������ʧ��  2����������ʧ��
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
* �������ƣ�ִ�б���ǰ������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ǰ�ñ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�лָ�ǰ������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ǰ�ûָ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�м�ʱ�ָ�ǰ������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ǰ�ü�ʱ�ָ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�б��ݷֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ı��ݷֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�лָ��ֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ļָ��ֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�й��طֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ĺ��طֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�й��طֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ĺ��طֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�м�ʱ�ָ��ֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ļ�ʱ�ָ��ֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�������ֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������������ֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ��ɾ���ֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ɾ���ֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�б�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ı�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�лָ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ļָ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�й�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ĺ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�й�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ĺ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�м�ʱ�ָ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ļ�ʱ�ָ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ����������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1����������������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ��ɾ������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1����������������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�б��ݺ�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ı��ݺ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�лָ���������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ļָ���������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ��������������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������������������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�й��غ�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������Ĺ��غ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ��ȡ�����غ�������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ȡ�����غ�������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�м�ʱ�ָ���������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1�������ļ�ʱ�ָ���������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ���ֹ������ɹ���������δ���͵����
* ǰ��������1��agent��������������� 2. ����δ���͵����
* check�㣺��ֹ������ɹ�
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
* �������ƣ���ֹ�������·��ɹ����������Ѿ����͵����
* ǰ��������1��agent��������������� 2. �����Ѿ����͵���� 3. �����ǰ״̬Ϊ������
* check�㣺��ֹ�������·��ɹ�
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
* �������ƣ�������ֹ����������
* ǰ��������1��������������agent������д���
* check�㣺������ֹ����������
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
* �������ƣ�������������������״̬�л������״̬
* ǰ��������1���������������� 2. ��������ϱ�״̬
* check�㣺������״̬�л�����
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
* �������ƣ�������������������״̬�л������״̬���������ʱ����Դ
* ǰ��������1���������������� 2. ��������ϱ�״̬
* check�㣺��Դ������
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

    /*======== ״̬��UNDEFINE�л���PrepareFailed ============*/
    subjob->m_data.status = 255;
    subjob->ChangeState(SubJobState::UNDEFINE);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::PrepareFailed);
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);

    /*======== ״̬��Running�л���SubJobComplete ============*/
    subjob->ChangeState(SubJobState::Running);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::SubJobComplete);
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);

    /*======== ״̬��Running�л���SubJobFailed ============*/
    subjob->ChangeState(SubJobState::Running);
    EXPECT_NE(nullptr, subjob->m_pluginUdTimer);
    subjob->ChangeState(SubJobState::SubJobFailed );
    EXPECT_EQ(nullptr, subjob->m_pluginUdTimer);
}


/*
* �������ƣ�����ʱ��ֹ�ӿ��·����ִ��
* ǰ��������1�������·�PauseJob�ӿڵ���
* check�㣺�ӿڷ��سɹ�
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
* �������ƣ�ɾ��qos����
* ǰ��������
* check�㣺�ӿڵ��óɹ�
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
* �������ƣ��Ƿ��ֶ�����
* ǰ��������
* check�㣺�ӿڵ��óɹ�
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
* �������ƣ���ԭ����ʽ�ָ������ļ�ϵͳ
* ǰ���������ָ�����
* check�㣺1������ִ�гɹ�
           2���·��ั������·��
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
* �������ƣ�ִ�и���У��ֽ�����
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1��������ɾ���ֽ�����ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�и���У������
* ǰ��������1��ͨ��ExternalPluinManager��ȡthrift�ӿ�ָ��
* check�㣺1����������������ָ�벻Ϊ�� 2 ����ִ������
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
* �������ƣ�ִ�нű�����ע������
* ǰ��������
* check�㣺1���ű���������ע��, �ű����سɹ� 2.�����ű���������
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
