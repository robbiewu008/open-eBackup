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
#include <vector>
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "config_reader/ConfigIniReader.h"
#include "ClientInvoke.h"
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "CancelLivemountJob.h"

namespace {
const std::string MODULE = "CANCEL_LIVEMOUNT";
using Defer = std::shared_ptr<void>;
constexpr int IO_TIME_OUT = 5; // ms
constexpr uint64_t DEFAULT_BACKUP_THREADS = 1;
constexpr uint64_t DEFAULT_SEGMENT_THRESHOLD = 60 * 1024 * 1024 * 1024ULL; // 默认卷分段阈值为60Gb
constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024;
constexpr uint32_t RETRY_INTERVAL_SECOND = 3;
const std::string ALARM_CODE_FAILED_DELETE_MACHINE = "0x6403400008";
const std::string ALARM_CODE_FAILED_DELETE_VOLUME = "0x6403400009";
const std::string SUBJOB_INDEX = "0";
}

namespace VirtPlugin {
bool CancelLivemountJob::InitHandlers()
{
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("LiveMountPara ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (m_cancelLivemountPara->copy.repositories.empty()) {
        ERRLOG("No repositories data, %s", m_taskInfo.c_str());
        return false;
    }
    // 保护对象Handler
    if (InitProtectEngineHandler(JobType::CANCELLIVEMOUNT) != SUCCESS) {
        ERRLOG("Initialize protect engine handler failed, %s", m_taskInfo.c_str());
        return false;
    }
    INFOLOG("Copy name is %s, copy id %s, %s", m_cancelLivemountPara->copy.name.c_str(),
        m_cancelLivemountPara->copy.id.c_str(), m_taskInfo.c_str());
    std::vector<StorageRepository> repoList = m_cancelLivemountPara->copy.repositories;
    for (const auto &repo : repoList) {
        if (repo.protocol == RepositoryProtocolType::S3 && m_cacheRepoHandler != nullptr) {
            INFOLOG("Init S3 repo handler success.%s", m_taskInfo.c_str());
            return true;
        }
        if (repo.repositoryType == RepositoryDataType::CACHE_REPOSITORY &&
                    !DoInitHandlers(repo, m_cacheRepoHandler, m_cacheRepoPath)) {
            ERRLOG("Init cache repo handler failed, %s", m_taskInfo.c_str());
            return false;
        } else if (repo.repositoryType == RepositoryDataType::DATA_REPOSITORY &&
                        !DoInitHandlers(repo, m_dataRepoHandler, m_dataRepoPath)) {
            ERRLOG("Init data repo handler failed, %s", m_taskInfo.c_str());
            return false;
        } else if (repo.repositoryType == RepositoryDataType::META_REPOSITORY &&
                        !DoInitHandlers(repo, m_metaRepoHandler, m_metaRepoPath)) {
            ERRLOG("Init meta repo handler failed, %s", m_taskInfo.c_str());
            return false;
        }
    }
    if (m_cacheRepoHandler == nullptr || m_dataRepoHandler == nullptr || m_metaRepoHandler == nullptr) {
        ERRLOG("Init repo handler failed, %s", m_taskInfo.c_str());
        return false;
    }
    INFOLOG("Init repo handler success, %s", m_taskInfo.c_str());
    return true;
}

bool CancelLivemountJob::InitRepo() const
{
    std::string cancelMountJobCachePath = m_cacheRepoPath + VIRT_PLUGIN_CACHE_CANCELMOUNTJOB_ROOT;
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (!m_cacheRepoHandler->Exists(cancelMountJobCachePath)) {
        DBGLOG("Creating restore cache directory: %s, %s", cancelMountJobCachePath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_cacheRepoHandler,
            cancelMountJobCachePath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create restore cache directory %s failed, %s", cancelMountJobCachePath.c_str(), m_taskInfo.c_str());
            return false;
        }
    }
    return true;
}

bool CancelLivemountJob::CommonInfoInit()
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("JobCommonInfo is null, %s", m_taskInfo.c_str());
        return false;
    }
    m_cancelLivemountPara = std::dynamic_pointer_cast<AppProtect::CancelLivemountJob>(m_jobCommonInfo->GetJobInfo());
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("Init m_cancelLivemountPara failed, %s", m_taskInfo.c_str());
        return false;
    }
    m_taskInfo = GetTaskId();
    m_jobId = m_cancelLivemountPara->jobId;
    if (!InitHandlers()) {
        ERRLOG("Init handlers failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (!InitRepo()) {
        ERRLOG("Init repo path failed, %s", m_taskInfo.c_str());
        return false;
    }
    m_liveMountVMMetaDataPath = m_metaRepoPath + VIRT_PLUGIN_LIVE_VM_INFO;
    return true;
}

bool CancelLivemountJob::CheckMainTaskStatusFileExist()
{
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    return m_cacheRepoHandler->Exists(mainTaskStatusInfoFile);
}

int CancelLivemountJob::LoadVMInfo(VMInfo &vmInfo)
{
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_liveMountVMMetaDataPath, vmInfo) != SUCCESS) {
        ERRLOG("Load file %s to struct failed, %s", m_liveMountVMMetaDataPath.c_str(), m_taskInfo.c_str());
        return FAILED;
    }

    INFOLOG("Load vm info success. VM name: %s, %s", vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int CancelLivemountJob::CheckBeforeUnmount()
{
    if (m_protectEngine->CheckBeforeUnmount() != SUCCESS) {
        ERRLOG("Check before unmount failed, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_POWER_OFF_MACHINE);
    return SUCCESS;
}

int CancelLivemountJob::PowerOffMachine()
{
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_DELETE_MACHINE);
    if (m_protectEngine->PowerOffMachine(m_vmInfo) != SUCCESS) {
        ERRLOG("Poweroff machine %s failed, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    ReportTaskLabel();
    DBGLOG("Power off machine(%s) success, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

// --------------------------------------- Generate sub task-----------------------------------------------------------
void CancelLivemountJob::SetGenerateJobStateMachine()
{
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_GEN_JOB_INIT)] =
        std::bind(&CancelLivemountJob::GenerateJobInit, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_PREHOOK)] =
        std::bind(&CancelLivemountJob::GenerateSubJobPreHook, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_DO_GENERATE_SUBJOB)] =
        std::bind(&CancelLivemountJob::DoGenerateSubJob, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_POSTHOOK)] =
        std::bind(&CancelLivemountJob::GenerateSubJobPostHook, this);
}

int CancelLivemountJob::GenerateJobInit()
{
    if (!CommonInfoInit()) {
        ERRLOG("Init base info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 存在主任务状态文件 则表明已经执行过当前任务了，直接返回成功
    if (CheckMainTaskStatusFileExist()) {
        INFOLOG("The main task has been executed.skip gen sub job. %s", m_taskInfo.c_str());
        m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_NONE);
        return SUCCESS;
    }
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_PREHOOK);
    return SUCCESS;
}

EXTER_ATTACK int CancelLivemountJob::GenerateSubJob()
{
    SetGenerateJobStateMachine();
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_GEN_JOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int CancelLivemountJob::GenMainTaskStatusToFile()
{
    std::string genMainTaskStatusInfoStr = "Sub job has been generated.";
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, mainTaskStatusInfoFile, genMainTaskStatusInfoStr) != SUCCESS) {
        ERRLOG("Failed to generate main task status to file, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Generate main task status to file success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int CancelLivemountJob::DoGenerateSubJob()
{
    SubJob subJob {};
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName(BUSINESS_SUB_JOB_NAME_PREFIX + SUBJOB_INDEX);
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    int priority = 1;
    subJob.__set_jobPriority(priority);
    SubJobExtendInfo eInfo;
    std::string infoStr = "";
    if (!Module::JsonHelper::StructToJsonString(eInfo, infoStr)) {
        ERRLOG("SubJob extend info format error, %s", m_taskInfo.c_str());
        return FAILED;
    }
    subJob.__set_jobInfo(infoStr);
    std::vector<SubJob> subJobs {};
    subJobs.emplace_back(subJob);

    if (!AddNewJobWithRetry(subJobs)) {
        ERRLOG("Add new job fail, %s", m_taskInfo.c_str());
        return FAILED;
    }
    auto ret = GenMainTaskStatusToFile();
    if (ret != SUCCESS) {
        WARNLOG("Failed to save main task status to file, %s", m_taskInfo.c_str());
    }

    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_POSTHOOK);
    return SUCCESS;
}

EXTER_ATTACK int CancelLivemountJob::ExecuteSubJob()
{
    int ret = ExecuteSubJobInner();
    if (ret != SUCCESS) {
        ReportTaskLabel();
    }
    ReportJobResult(ret, "ExecuteSubJob finish.", m_completedDataSize);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int CancelLivemountJob::ExecuteSubJobInner()
{
    if (!CommonInfoInit()) {
        ERRLOG("Failed to Init common info, %s", m_taskInfo.c_str());
        return FAILED;
    }
    int ret = FAILED;
    if (m_subJobInfo == nullptr) {
        ERRLOG("SubJobInfo ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    SubJobStateInit();
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_PREHOOK);
    ret = RunStateMachine();
    // 清理资源
    if ((SubTaskClean() != SUCCESS) || (ret != SUCCESS)) {
        ERRLOG("Failed to clean task, ret=%d, %s", ret, m_taskInfo.c_str());
        return FAILED;
    }
    return ret;
}

void CancelLivemountJob::SubJobStateInit()
{
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_PREHOOK)] =
        std::bind(&CancelLivemountJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_SUB_JOB_INIT)] =
        std::bind(&CancelLivemountJob::SubTaskInitialize, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_CHECK_BEFORE_UNMOUNT)] =
        std::bind(&CancelLivemountJob::CheckBeforeUnmount, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_POWER_OFF_MACHINE)] =
        std::bind(&CancelLivemountJob::PowerOffMachine, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_DELETE_MACHINE)] =
        std::bind(&CancelLivemountJob::DeleteVirtualMachine, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_CANCEL_MOUNT)] =
        std::bind(&CancelLivemountJob::CancelNasMount, this);
    m_stateHandles[static_cast<int>(CancelLivemountJobStates::STATE_EXEC_POSTHOOK)] =
        std::bind(&CancelLivemountJob::ExecuteSubJobPostHook, this);
}

int CancelLivemountJob::SubTaskInitialize()
{
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_CHECK_BEFORE_UNMOUNT);

    /* get vm info */
    if (LoadVMInfo(m_vmInfo) != SUCCESS) {
        ERRLOG("Load machine metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_protectEngine != nullptr) {
        m_protectEngine->ClearLabel();
    }
    return SUCCESS;
}

int CancelLivemountJob::DeleteVirtualMachine()
{
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_CANCEL_MOUNT);
    if (m_protectEngine->DeleteMachine(m_vmInfo) != SUCCESS) {
        ERRLOG("Delete machine %s failed, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }
    INFOLOG("Delete machine %s success, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int CancelLivemountJob::CancelNasMount()
{
    m_nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_POSTHOOK);
    if (m_protectEngine->CancelLiveMount(m_vmInfo) != SUCCESS) {
        ERRLOG("Cancel tmp mount datastore of machine %s failed, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }
    INFOLOG("Cancel tmp mount datastore of machine %s success, %s", m_vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int CancelLivemountJob::SubTaskClean()
{
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_cacheRepoHandler->Exists(m_liveMountVMMetaDataPath)) {
        m_cacheRepoHandler->Remove(m_liveMountVMMetaDataPath);
    }
    return SUCCESS;
}

int CancelLivemountJob::GenerateSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.nextState = static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_DO_GENERATE_SUBJOB);
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.postHookState = static_cast<int>(CancelLivemountJobStates::STATE_GENERATE_POSTHOOK);
    return ExecHook(para);
}

int CancelLivemountJob::GenerateSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.nextState = static_cast<int>(CancelLivemountJobStates::STATE_NONE);
    return ExecHook(para);
}

int CancelLivemountJob::ExecuteSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_SUB_JOB_INIT);
    para.postHookState = static_cast<int>(CancelLivemountJobStates::STATE_EXEC_POSTHOOK);
    return ExecHook(para);
}

int CancelLivemountJob::ExecuteSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(CancelLivemountJobStates::STATE_NONE);
    return ExecHook(para);
}
}