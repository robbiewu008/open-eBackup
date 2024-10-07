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
#include "ProtectServiceImp.h"
#include "JobFactoryBase.h"
#include "JobExecution.h"
#include "JobMgr.h"
#include "OpenLibMgr.h"
#include "log/Log.h"
#include "param_checker/ParamChecker.h"
#include "JsonTransUtil.h"
#include "Utils.h"

using namespace AppProtect;
using namespace std;
using namespace common::jobmanager;

using CreateFactoryFun = JobFactoryBase* ();
using CheckBackupJobTypeFun = void (ActionResult& returnValue, const BackupJob& job);
using AllowBackupInLocalNodeFun = void (ActionResult& returnValue, const BackupJob& job, const BackupLimit::type limit);
using AllowBackupSubJobInLocalNodeFun = void (ActionResult& returnValue, const BackupJob& job, const SubJob& subJob);
using AllowRestoreInLocalNodeFun = void (ActionResult& returnValue, const RestoreJob& job);
using AllowRestoreSubJobInLocalNodeFun = void (ActionResult& returnValue, const RestoreJob& job, const SubJob& subJob);
using QueryJobPermissionFun = void (JobPermission& returnJobPermission, const ApplicationEnvironment& appEnv,
    const Application& application);
using DeliverTaskStatusFun = void (ActionResult& returnValue, const std::string& status, const std::string& jobId,
    const std::string& script);
using AllowCheckCopyInLocalNodeFun = void (ActionResult& returnValue, const CheckCopyJob& job);
using AllowCheckCopySubJobInLocalNodeFun = void (ActionResult& returnValue, const CheckCopyJob& job,
    const SubJob& subJob);

namespace {
    constexpr auto MODULE = "ProtectServiceImp";
    const std::string PREREQUISITE = "PRE";
    const std::string GENERATE = "GEN";
    const std::string POST = "PST";
    constexpr auto FLR_RESTORE_TYPE_ORIGIN_VALUE = "original";
    constexpr int MAX_ERR_MSG_LEN = 256;

    template<typename T>
    int CreateJob(AppProtect::ActionResult& returnValue, std::shared_ptr<BasicJob>& jobPtr,
        const T& job, JobType jobType)
    {
        HCP_Log(DEBUG, MODULE) << "Enter CreateJob" <<HCPENDLOG;
        // 封装job信息
        std::shared_ptr<ThriftDataBase> data = std::make_shared<T>(job);
        auto jobInfo = std::make_shared<JobCommonInfo>(data);

        // 通过打开动态库，获取工厂对象
        auto fun = OpenLibMgr::GetInstance().GetObj<CreateFactoryFun>("CreateFactory");
        if (fun == nullptr) {
            char errMsg[MAX_ERR_MSG_LEN] = {0};
            HCP_Log(ERR, MODULE) << "Get CreateFactory function failed: " <<
                Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
            return Module::FAILED;
        }
        HCP_Log(INFO, MODULE) << "Get CreateFactory success" <<HCPENDLOG;
        JobFactoryBase* jobFactory = fun();
        if (jobFactory == nullptr) {
            char errMsg[MAX_ERR_MSG_LEN] = {0};
            HCP_Log(ERR, MODULE) << "CreateFactory failed error:" <<
                Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
            returnValue.__set_code(INNER_ERROR);
            return Module::FAILED;
        }
        // 获取job对象
        jobPtr = jobFactory->CreateJob(jobInfo, jobType);
        if (jobPtr == nullptr) {
            HCP_Log(ERR, MODULE) << "job Ptr is null" <<HCPENDLOG;
            returnValue.__set_code(INNER_ERROR);
            return Module::FAILED;
        }
        HCP_Log(INFO, MODULE) << "Exit CreateJob success" <<HCPENDLOG;
        returnValue.__set_code(0);
        return Module::SUCCESS;
    }

    int ExcuteJob(AppProtect::ActionResult& returnValue, std::shared_ptr<BasicJob> jobPtr,
        const std::string& jobId, OperType type)
    {
        // 执行异步任务，创建线程，JobMgr纳入管理
        HCP_Log(DEBUG, MODULE) << "Enter excute job" << HCPENDLOG;
        JobExecution jobExecution;
        int ret = jobExecution.ExecuteJob(returnValue, jobPtr, jobId, type);
        if (ret != Module::SUCCESS) {
            returnValue.__set_code(INNER_ERROR);
            HCP_Log(ERR, MODULE) << "Excute job failed" << HCPENDLOG;
            return Module::FAILED;
        }
        returnValue.__set_code(0);
        HCP_Log(INFO, MODULE) << "Exit excute job success" HCPENDLOG;
        return Module::SUCCESS;
    }
}

ProtectServiceImp::ProtectServiceImp()
{}

ProtectServiceImp::~ProtectServiceImp()
{}


EXTER_ATTACK void ProtectServiceImp::AsyncAbortJob(ActionResult& returnValue, const std::string& jobId,
    const std::string& subJobId, const std::string& appType)
{
    (void)appType;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncAbortJob jobId: " << WIPE_SENSITIVE(jobId) << " subJobId: "
        << WIPE_SENSITIVE(subJobId) << HCPENDLOG;
    JobMgr::GetInstance().AsyncAbortJob(jobId, subJobId);
    returnValue.__set_code(0);
    HCP_Log(INFO, MODULE) << "Exit AsyncAbortJob jobId: " << WIPE_SENSITIVE(jobId)
        << " subJobId:" << WIPE_SENSITIVE(subJobId) << HCPENDLOG;
    return;
}

EXTER_ATTACK void ProtectServiceImp::PauseJob(ActionResult& returnValue, const std::string& jobId,
    const std::string& subJobId, const std::string& appType)
{
    (void)appType;
    HCP_Log(DEBUG, MODULE) << "Enter PauseJob jobId: " << WIPE_SENSITIVE(jobId) << " subJobId: "
        << WIPE_SENSITIVE(subJobId) << HCPENDLOG;
    JobMgr::GetInstance().PauseJob(jobId, subJobId);
    returnValue.__set_code(0);
    HCP_Log(INFO, MODULE) << "Exit PauseJob jobId: " << WIPE_SENSITIVE(jobId)
        << " subJobId: " << WIPE_SENSITIVE(subJobId) << HCPENDLOG;
    return;
}

EXTER_ATTACK void ProtectServiceImp::CheckBackupJobType(ActionResult& returnValue, const BackupJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter CheckBackupJobType" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<CheckBackupJobTypeFun>("CheckBackupJobType");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get CheckBackupJobType function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck("BackupJob", StructToJson(job), returnValue)) {
        return;
    }
    fun(returnValue, job);
    HCP_Log(INFO, MODULE) << "Exit CheckBackupJobType" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::QueryJobPermission(JobPermission& returnJobPermission,
    const ApplicationEnvironment& appEnv, const Application& application)
{
    HCP_Log(DEBUG, MODULE) << "Enter QueryJobPermission" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<QueryJobPermissionFun>("QueryJobPermission");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get QueryJobPermission function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }

    ParamCheck({{"ApplicationEnvironment", StructToJson(appEnv)}, {"Application", StructToJson(application)}});
    fun(returnJobPermission, appEnv, application);
    HCP_Log(INFO, MODULE) << "Exit QueryJobPermission" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AllowBackupInLocalNode(ActionResult& returnValue,
    const BackupJob& job, const BackupLimit::type limit)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowBackupInLocalNode" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AllowBackupInLocalNodeFun>("AllowBackupInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowBackupInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck("BackupJob", StructToJson(job), returnValue)) {
        return;
    }
    fun(returnValue, job, limit);
    HCP_Log(INFO, MODULE) << "Exit AllowBackupInLocalNode" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AllowBackupSubJobInLocalNode(
    ActionResult& returnValue, const BackupJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowBackupSubJobInLocalNode" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AllowBackupSubJobInLocalNodeFun>("AllowBackupSubJobInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowBackupSubJobInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck({{"BackupJob", StructToJson(job)}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }
    fun(returnValue, job, subJob);
    HCP_Log(INFO, MODULE) << "Exit AllowBackupSubJobInLocalNode" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AsyncBackupPrerequisite(ActionResult& returnValue, const BackupJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncBackupPrerequisite" << HCPENDLOG;
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncBackupPrerequisite, parameter:" <<
        WIPE_SENSITIVE(backupJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("BackupJob", backupJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BackupJob>(returnValue, jobPtr, job, JobType::BACKUP) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + PREREQUISITE;
    HCP_Log(INFO, MODULE) << "AsyncBackupPrerequisite jobid: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::PRE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncBackupGenerateSubJob(
    ActionResult& returnValue, const BackupJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncBackupGenerateSubJob" << HCPENDLOG;
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncBackupGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(backupJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("BackupJob", backupJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BackupJob>(returnValue, jobPtr, job, JobType::BACKUP) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncBackupGenerateSubJob jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncExecuteBackupSubJob(ActionResult& returnValue,
    const BackupJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncExecuteBackupSubJob" << HCPENDLOG;
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncExecuteBackupSubJob, parameter:" <<
        WIPE_SENSITIVE(backupJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"BackupJob", backupJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BackupJob>(returnValue, jobPtr, job, JobType::BACKUP) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    jobPtr->SetJobId(subJobId);
    HCP_Log(INFO, MODULE) << "AsyncExecuteBackupSubJob parent JobId:" << parentJobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncExecuteBackupSubJob subJobId:" << subJobId << HCPENDLOG;

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncBackupPostJob(ActionResult& returnValue, const BackupJob& job,
    const AppProtect::SubJob& subJob, const JobResult::type backupJobResult)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncBackupPostJob" << HCPENDLOG;
    Json::Value backupJobStr;
    StructToJson(job, backupJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncBackupPostJob, parameter:" <<
        WIPE_SENSITIVE(backupJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"BackupJob", backupJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BackupJob>(returnValue, jobPtr, job, JobType::BACKUP) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    jobPtr->SetPostJobResultType(backupJobResult);
    std::string jobId = job.jobId;
    jobPtr->SetJobId(jobId);
    HCP_Log(INFO, MODULE) << "AsyncBackupPostJob JobId:" << jobId << HCPENDLOG;
    std::string mgrJobId = jobId + "_" + POST;

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::POST);
}

EXTER_ATTACK void ProtectServiceImp::AllowRestoreInLocalNode(ActionResult& returnValue, const RestoreJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowRestoreInLocalNode" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AllowRestoreInLocalNodeFun>("AllowRestoreInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowRestoreInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck("RestoreJob", StructToJson(job), returnValue)) {
        return;
    }
    fun(returnValue, job);
    HCP_Log(INFO, MODULE) << "Exit AllowRestoreInLocalNode" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AllowRestoreSubJobInLocalNode(
    ActionResult& returnValue, const RestoreJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowRestoreSubJobInLocalNode" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AllowRestoreSubJobInLocalNodeFun>("AllowRestoreSubJobInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowRestoreSubJobInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck({{"RestoreJob", StructToJson(job)}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }
    fun(returnValue, job, subJob);
    HCP_Log(INFO, MODULE) << "Exit AllowRestoreSubJobInLocalNode" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AsyncRestorePrerequisite(ActionResult& returnValue, const RestoreJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncRestorePrerequisite" << HCPENDLOG;
    Json::Value restoreJobStr;
    StructToJson(job, restoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncRestorePrerequisite, parameter:" <<
        WIPE_SENSITIVE(restoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("RestoreJob", restoreJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + PREREQUISITE;
    HCP_Log(INFO, MODULE) << "AsyncRestorePrerequisite jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::PRE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncRestoreGenerateSubJob(
    ActionResult& returnValue, const  AppProtect::RestoreJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncRestoreGenerateSubJob" << HCPENDLOG;
    Json::Value restoreJobStr;
    StructToJson(job, restoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncRestoreGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(restoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("RestoreJob", restoreJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncRestoreGenerateSubJob jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncExecuteRestoreSubJob(ActionResult& returnValue,
    const RestoreJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncExecuteRestoreSubJob" << HCPENDLOG;
    Json::Value restoreJobStr;
    StructToJson(job, restoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncExecuteRestoreSubJob, parameter:" <<
        WIPE_SENSITIVE(restoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"RestoreJob", restoreJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    jobPtr->SetJobId(subJobId);
    HCP_Log(INFO, MODULE) << "AsyncExecuteRestoreSubJob parent JobId:" << parentJobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncExecuteRestoreSubJob subJobId:" << subJobId << HCPENDLOG;

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncRestorePostJob(ActionResult& returnValue, const RestoreJob& job,
    const SubJob& subJob, const JobResult::type restoreJobResult)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncRestorePostJob" << HCPENDLOG;
    Json::Value restoreJobStr;
    StructToJson(job, restoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncRestorePostJob, parameter:" <<
        WIPE_SENSITIVE(restoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"RestoreJob", restoreJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    HCP_Log(INFO, MODULE) << "AsyncRestorePostJob parent JobId:" << parentJobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncRestorePostJob subJobId:" << subJobId << HCPENDLOG;

    jobPtr->SetPostJobResultType(restoreJobResult);
    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + POST;
    HCP_Log(INFO, MODULE) << "AsyncRestorePostJob JobId:" << jobId << HCPENDLOG;
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::POST);
}

EXTER_ATTACK void ProtectServiceImp::AsyncLivemountGenerateSubJob(
    ActionResult& returnValue, const LivemountJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncLivemountGenerateSubJob" << HCPENDLOG;
    Json::Value livemountJobStr;
    StructToJson(job, livemountJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncLivemountGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(livemountJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("LivemountJob", livemountJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<LivemountJob>(returnValue, jobPtr, job, JobType::LIVEMOUNT) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncLivemountGenerateSubJob jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncExecuteLivemountSubJob(ActionResult& returnValue,
    const LivemountJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncExecuteLivemountSubJob" << HCPENDLOG;
    Json::Value liveMountJobStr;
    StructToJson(job, liveMountJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncExecuteLivemountSubJob, parameter:" <<
        WIPE_SENSITIVE(liveMountJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"LivemountJob", liveMountJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<LivemountJob>(returnValue, jobPtr, job, JobType::LIVEMOUNT) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    jobPtr->SetJobId(subJobId);
    HCP_Log(INFO, MODULE) << "AsyncExecuteLivemountSubJob parent JobId:" << parentJobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncExecuteLivemountSubJob subJobId:" << subJobId << HCPENDLOG;

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncCancelLivemountGenerateSubJob(ActionResult& returnValue,
    const CancelLivemountJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncCancelLivemountGenerateSubJob" << HCPENDLOG;

    Json::Value cancelLiveMountJobStr;
    StructToJson(job, cancelLiveMountJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncCancelLivemountGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(cancelLiveMountJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("CancelLivemountJob", cancelLiveMountJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<CancelLivemountJob>(returnValue, jobPtr, job, JobType::CANCELLIVEMOUNT) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncCancelLivemountGenerateSubJob jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncExecuteCancelLivemountSubJob(ActionResult& returnValue,
    const CancelLivemountJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncExecuteCancelLivemountSubJob" << HCPENDLOG;

    Json::Value cancelLiveMountJobStr;
    StructToJson(job, cancelLiveMountJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncExecuteLivemountSubJob, parameter:" <<
        WIPE_SENSITIVE(cancelLiveMountJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"CancelLivemountJob", cancelLiveMountJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<CancelLivemountJob>(returnValue, jobPtr, job, JobType::CANCELLIVEMOUNT) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    jobPtr->SetJobId(subJobId);
    HCP_Log(INFO, MODULE) << "AsyncExecuteCancelLivemountSubJob subJobId:" << subJobId <<
        " parent JobId:" << parentJobId << HCPENDLOG;

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncInstantRestorePrerequisite(ActionResult& returnValue, const RestoreJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncInstantRestorePrerequisite" << HCPENDLOG;
    Json::Value instantRestoreJobStr;
    StructToJson(job, instantRestoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncInstantRestorePrerequisite, parameter:" <<
        WIPE_SENSITIVE(instantRestoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("RestoreJob", instantRestoreJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::INSTANT_RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + PREREQUISITE;
    HCP_Log(INFO, MODULE) << "AsyncInstantRestorePrerequisite jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::PRE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncInstantRestoreGenerateSubJob(
    ActionResult& returnValue, const AppProtect::RestoreJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncInstantRestoreGenerateSubJob" << HCPENDLOG;
    Json::Value instantRestoreJobStr;
    StructToJson(job, instantRestoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncInstantRestoreGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(instantRestoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("RestoreJob", instantRestoreJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::INSTANT_RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncInstantRestoreGenerateSubJob jobId: " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncExecuteInstantRestoreSubJob(
    ActionResult& returnValue, const RestoreJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncExecuteInstantRestoreSubJob" << HCPENDLOG;
    Json::Value instantRestoreJobStr;
    StructToJson(job, instantRestoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncExecuteInstantRestoreSubJob, parameter:" <<
        WIPE_SENSITIVE(instantRestoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"RestoreJob", instantRestoreJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::INSTANT_RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    jobPtr->SetJobId(subJobId);
    HCP_Log(INFO, MODULE) << "AsyncExecuteInstantRestoreSubJob parent JobId:" << parentJobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncExecuteInstantRestoreSubJob subJobId:" << subJobId << HCPENDLOG;

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncInstantRestorePostJob(
    ActionResult& returnValue, const RestoreJob& job, const SubJob& subJob, const JobResult::type restoreJobResult)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncInstantRestorePostJob" << HCPENDLOG;
    Json::Value instantRestoreJobStr;
    StructToJson(job, instantRestoreJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncInstantRestorePostJob, parameter:" <<
        WIPE_SENSITIVE(instantRestoreJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck({{"RestoreJob", instantRestoreJobStr}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<RestoreJob>(returnValue, jobPtr, job, JobType::INSTANT_RESTORE) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string jobId = job.jobId;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);
    std::string subJobId = subJob.subJobId;
    HCP_Log(INFO, MODULE) << "AsyncInstantRestorePostJob parent JobId:" << jobId << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "AsyncInstantRestorePostJob subJobId:" << subJobId << HCPENDLOG;

    jobPtr->SetPostJobResultType(restoreJobResult);
    std::string mgrJobId = jobId + "_" + POST;

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::POST);
}

EXTER_ATTACK void ProtectServiceImp::AsyncBuildIndexGenerateSubJob(
    ActionResult& returnValue, const BuildIndexJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncBuildIndexGenerateSubJob" << HCPENDLOG;
    Json::Value buildIndexJobStr;
    StructToJson(job, buildIndexJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncBuildIndexGenerateSubJob, parameter:" <<
        WIPE_SENSITIVE(buildIndexJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("BuildIndexJob", buildIndexJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BuildIndexJob>(returnValue, jobPtr, job, JobType::INDEX) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncBuildIndexGenerateSubJob jobId : " << jobId << HCPENDLOG;
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncBuildIndexSubJob(ActionResult& returnValue,
    const BuildIndexJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncBuildIndexSubJob" << HCPENDLOG;
    Json::Value buildIndexJobStr;
    StructToJson(job, buildIndexJobStr);
    HCP_Log(DEBUG, MODULE) << "AsyncBuildIndexSubJob, parameter:" <<
        WIPE_SENSITIVE(buildIndexJobStr.toStyledString()) << HCPENDLOG;

    if (!ParamCheck("BuildIndexJob", buildIndexJobStr, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<BuildIndexJob>(returnValue, jobPtr, job, JobType::INDEX) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    HCP_Log(INFO, MODULE) << "AsyncBuildIndexSubJob sub jobId : " << subJobId << HCPENDLOG;
    jobPtr->SetJobId(subJobId);

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncDelCopyGenerateSubJob(
    ActionResult& returnValue, const DelCopyJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    HCP_Log(DEBUG, MODULE) << "Enter AsyncDelCopyGenerateSubJob" << HCPENDLOG;

    if (!ParamCheck("DelCopyJob", StructToJson(job), returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<DelCopyJob>(returnValue, jobPtr, job, JobType::DELCOPY) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    std::string mgrJobId = jobId + "_" + GENERATE;
    HCP_Log(INFO, MODULE) << "AsyncDelCopyGenerateSubJob jobId : " << jobId << HCPENDLOG;
    jobPtr->SetParentJobId(jobId);
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncDelCopySubJob(ActionResult& returnValue,
    const DelCopyJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AsyncDelCopySubJob" << HCPENDLOG;

    if (!ParamCheck({{"DelCopyJob", StructToJson(job)}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<DelCopyJob>(returnValue, jobPtr, job, JobType::DELCOPY) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    HCP_Log(INFO, MODULE) << "AsyncDelCopySubJob sub jobId : " << subJobId << HCPENDLOG;
    jobPtr->SetJobId(subJobId);

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
}

EXTER_ATTACK void ProtectServiceImp::AsyncCheckCopyGenerateSubJob(ActionResult& returnValue,
    const CheckCopyJob& job, const int32_t nodeNum)
{
    (void)nodeNum;
    INFOLOG("Enter AsyncCheckCopyGenerateSubJob");

    if (!ParamCheck("CheckCopyJob", StructToJson(job), returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<CheckCopyJob>(returnValue, jobPtr, job, JobType::CHECK_COPY) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    std::string jobId = job.jobId;
    jobPtr->SetParentJobId(jobId);
    std::string mgrJobId = jobId + "_" + GENERATE;
    INFOLOG("AsyncCheckCopyGenerateSubJob jobId : %s .", jobId.c_str());
    jobPtr->SetJobId(jobId);

    ExcuteJob(returnValue, jobPtr, mgrJobId, OperType::GENERATE);
    return;
}

EXTER_ATTACK void ProtectServiceImp::AsyncCheckCopySubJob(ActionResult& returnValue,
    const CheckCopyJob& job, const SubJob& subJob)
{
    INFOLOG("Enter AsyncCheckCopySubJob");

    if (!ParamCheck({{"CheckCopyJob", StructToJson(job)}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    std::shared_ptr<BasicJob> jobPtr;
    if (CreateJob<CheckCopyJob>(returnValue, jobPtr, job, JobType::CHECK_COPY) != Module::SUCCESS) {
        returnValue.__set_code(INNER_ERROR);
        return;
    }

    jobPtr->SetSubJob(std::make_shared<SubJob>(subJob));
    std::string parentJobId = job.jobId;
    jobPtr->SetParentJobId(parentJobId);
    std::string subJobId = subJob.subJobId;
    INFOLOG("AsyncCheckCopySubJob sub jobId : %s .", subJobId.c_str());
    jobPtr->SetJobId(subJobId);

    ExcuteJob(returnValue, jobPtr, subJobId, OperType::EXECUTE);
    return;
}

EXTER_ATTACK void ProtectServiceImp::DeliverTaskStatus(ActionResult& returnValue,
    const std::string& status, const std::string& jobId, const std::string& script)
{
    INFOLOG("Enter DeliverTaskStatus");
    auto fun = OpenLibMgr::GetInstance().GetObj<DeliverTaskStatusFun>("DeliverTaskStatus");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get DeliverTaskStatus function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    fun(returnValue, status, jobId, script);
    HCP_Log(INFO, MODULE) << "Exit DeliverTaskStatus" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AllowCheckCopyInLocalNode(ActionResult& returnValue, const CheckCopyJob& job)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowCheckCopyInLocalNode" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AllowCheckCopyInLocalNodeFun>("AllowCheckCopyInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowCheckCopyInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    if (!ParamCheck("CheckCopyJob", StructToJson(job), returnValue)) {
        return;
    }
    fun(returnValue, job);
    HCP_Log(INFO, MODULE) << "Exit AllowCheckCopyInLocalNode" << HCPENDLOG;
}

EXTER_ATTACK void ProtectServiceImp::AllowCheckCopySubJobInLocalNode(ActionResult& returnValue,
    const CheckCopyJob& job, const SubJob& subJob)
{
    HCP_Log(DEBUG, MODULE) << "Enter AllowCheckCopySubJobInLocalNode" << HCPENDLOG;

    if (!ParamCheck({{"CheckCopyJob", StructToJson(job)}, {"SubJob", StructToJson(subJob)}}, returnValue)) {
        return;
    }

    auto fun = OpenLibMgr::GetInstance().GetObj<AllowCheckCopySubJobInLocalNodeFun>("AllowCheckCopySubJobInLocalNode");
    if (fun == nullptr) {
        returnValue.__set_code(INNER_ERROR);
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AllowCheckCopySubJobInLocalNode function failed: " <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
        return;
    }
    fun(returnValue, job, subJob);
    HCP_Log(INFO, MODULE) << "Exit AllowCheckCopySubJobInLocalNode" << HCPENDLOG;
}
