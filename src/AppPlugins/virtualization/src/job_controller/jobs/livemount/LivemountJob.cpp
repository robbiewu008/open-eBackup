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
#include "LivemountJob.h"
#include <vector>
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "config_reader/ConfigIniReader.h"
#include "ClientInvoke.h"
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "job_controller/jobs/restore/RestoreJob.h"

namespace {
const std::string MODULE = "LIVEMOUNT";
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

bool LivemountJob::CommonInfoInit()
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("JobCommonInfo is null, %s", m_taskInfo.c_str());
        return false;
    }
    m_liveMountPara = std::dynamic_pointer_cast<AppProtect::LivemountJob>(m_jobCommonInfo->GetJobInfo());
    if (m_liveMountPara == nullptr) {
        ERRLOG("Init m_liveMountPara failed, %s", m_taskInfo.c_str());
        return false;
    }
    m_taskInfo = GetTaskId();
    if (!InitHandlers()) {
        ERRLOG("Init handlers failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (!InitRepo()) {
        ERRLOG("Init repo path failed, %s", m_taskInfo.c_str());
        return false;
    }
    return true;
}

EXTER_ATTACK int LivemountJob::GenerateSubJob()
{
    SetGenerateJobStateMachine();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int LivemountJob::PostClean()
{
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_cacheRepoHandler->Exists(m_newVMMetaDataPath)) {
        m_cacheRepoHandler->Remove(m_newVMMetaDataPath);
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK);
    return SUCCESS;
}

int LivemountJob::DeleteLiveMachine() // TO DO: 卸载挂载/失败清理
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK);
    return SUCCESS;
}

bool LivemountJob::InitHandlers()
{
    if (m_liveMountPara == nullptr) {
        ERRLOG("LiveMountPara ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (m_liveMountPara->copy.repositories.empty()) {
        ERRLOG("No repositories data, %s", m_taskInfo.c_str());
        return false;
    }
    // 保护对象Handler
    if (InitProtectEngineHandler(JobType::LIVEMOUNT) != SUCCESS) {
        ERRLOG("Initialize protect engine handler failed, %s", m_taskInfo.c_str());
        return false;
    }
    INFOLOG("Copy name is %s, copy id %s, %s", m_liveMountPara->copy.name.c_str(),
        m_liveMountPara->copy.id.c_str(), m_taskInfo.c_str());
    std::vector<StorageRepository> repoList = m_liveMountPara->copy.repositories;
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

bool LivemountJob::InitRepo() const
{
    std::string livemountJobCachePath = m_cacheRepoPath + VIRT_PLUGIN_CACHE_LIVEMOUNTJOB_ROOT;
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    if (!m_cacheRepoHandler->Exists(livemountJobCachePath)) {
        DBGLOG("Creating restore cache directory: %s, %s", livemountJobCachePath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_cacheRepoHandler,
            livemountJobCachePath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create restore cache directory %s failed, %s", livemountJobCachePath.c_str(), m_taskInfo.c_str());
            return false;
        }
    }
    return true;
}

bool LivemountJob::LoadVmMetaData()
{
    std::string vmmetaDataPath = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmmetaDataPath, m_copyVm) != SUCCESS) {
        ERRLOG("Failed to read file %s, %s", vmmetaDataPath.c_str(), m_taskInfo.c_str());
        return false;
    }
    return true;
}

bool LivemountJob::LoadMetaData()
{
    if (!LoadVmMetaData()) {
        return false;
    }
    return true;
}

EXTER_ATTACK int LivemountJob::ExecuteSubJob()
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

int LivemountJob::ExecuteSubJobInner()
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
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK);
    ret = RunStateMachine();
    // 清理资源
    if ((SubTaskClean() != SUCCESS) || (ret != SUCCESS)) {
        ERRLOG("Failed to clean task, ret=%d, %s", ret, m_taskInfo.c_str());
        return FAILED;
    }
    return ret;
}

void LivemountJob::SubJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK)] =
        std::bind(&LivemountJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_INITIALIZE)] =
        std::bind(&LivemountJob::SubTaskInitialize, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME)] =
        std::bind(&LivemountJob::SubTaskExecute, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POWERON_MACHINE)] =
        std::bind(&LivemountJob::SubJobPowerOnMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK)] =
        std::bind(&LivemountJob::ExecuteSubJobPostHook, this);
}

int LivemountJob::SubTaskInitialize()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME);
    DBGLOG("extendInfo %s extendInfo, copy %s copy", m_liveMountPara->extendInfo.c_str(),
        m_liveMountPara->copy.extendInfo.c_str());
    Json::Value advancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_liveMountPara->extendInfo, advancePara)) {
        ERRLOG("Convert %s failed, %s", m_liveMountPara->jobParam.advanceParams.c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }

    if (!LoadMetaData()) {
        ERRLOG("Load meta data failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    if (advancePara.isMember("livemountType")) {
        std::size_t* startPos = nullptr;
        int base = 10;
        m_jobType = LivemountType(std::stoi(advancePara["livemountType"].asString(), startPos, base));
        return SUCCESS;
    } else {
        m_jobType = LivemountType::MOUNT;
        return SUCCESS;
    }
    ERRLOG("AdvancePara miss param %s, %s", WIPE_SENSITIVE(m_liveMountPara->jobParam.advanceParams).c_str(),
        m_taskInfo.c_str());
    return FAILED;
}

int LivemountJob::SubTaskExecute()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POWERON_MACHINE);
    if (m_protectEngine == nullptr) {
        ERRLOG("ProtectEngine ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (CheckBeforeMount() != SUCCESS) {
        ERRLOG("CheckBeforeMount failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    int iRet = 0;
    if (m_jobType == LivemountType::MOUNT_CANCEL) {
        iRet = m_protectEngine->CancelLiveMount(m_copyVm);
        return iRet;
    }
    VMInfo newVm;
    if (m_jobType == LivemountType::RESTORE || m_jobType == LivemountType::MOUNT) {
        iRet = m_protectEngine->CreateLiveMount(m_copyVm, newVm);
        if (iRet != SUCCESS) {
            ERRLOG("CreateLiveMount failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        m_newVm = newVm;
        iRet = SaveVMInfo(newVm);
        if (iRet != SUCCESS) {
            ERRLOG("SaveVMInfo failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    }
    if (m_jobType == LivemountType::RESTORE || m_jobType == LivemountType::MIGRATION) {
        bool isRestore = m_jobType == LivemountType::RESTORE ? true : false;
        iRet = m_protectEngine->MigrateLiveVolume(newVm);
    }
    return iRet;
}

int LivemountJob::CheckBeforeMount()
{
    INFOLOG("Enter");
    if (m_protectEngine->CheckBeforeMount() != SUCCESS) {
        ERRLOG("Check before recover failed, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    std::string whiteListIpStr;
    if (m_protectEngine->GetWhiteListForLivemount(whiteListIpStr) != SUCCESS) {
        ERRLOG("GetWhiteListForLivemount, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    return AddWhiteList(whiteListIpStr) ? SUCCESS : FAILED;
}

int LivemountJob::ReportCopy()
{
    DBGLOG("In ReportCopy(), %s", m_taskInfo.c_str());
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        Copy image;
        if (FormatBackupCopy(image) != SUCCESS) {
            ERRLOG("Format backup copy failed, %s", m_taskInfo.c_str());
            return FAILED;
        }

        ActionResult returnValue;
        JobService::ReportCopyAdditionalInfo(returnValue, GetParentJobId(), image);
        if (returnValue.code != SUCCESS) {
            ERRLOG("Exit ReportCopyAdditionalInfo Failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
    } else {
        INFOLOG("Job execution failed in previous stage, skipping report copy, %s", m_taskInfo.c_str());
    }
    return SUCCESS;
}

int LivemountJob::FormatBackupCopy(Copy &copy)
{
    std::string sha256FileState = "";
    std::string vmInfoFile = m_metaRepoPath + VIRT_PLUGIN_LIVE_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmInfoFile, m_newVm) != SUCCESS) {
        ERRLOG("Load previous metadata failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    CopyExtendInfo info;
    std::string strInfo;
    info.m_copyVerifyFile = sha256FileState;
    info.m_volList = m_newVm.m_volList;
    info.m_interfaceList = m_newVm.m_interfaceList;
    if (!Module::JsonHelper::StructToJsonString(info, strInfo)) {
        ERRLOG("Exit PostJob StructToJsonString Failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("FormatBackupCopy extendInfo: %s, %s", WIPE_SENSITIVE(strInfo).c_str(), m_taskInfo.c_str());
    copy.__set_extendInfo(strInfo);
    return SUCCESS;
}

int LivemountJob::SaveVMInfo(const VMInfo &vmInfo)
{
    std::string vmInfoStr;
    VMInfo tempVm = vmInfo;
    if (!Module::JsonHelper::StructToJsonString(tempVm, vmInfoStr)) {
        ERRLOG("Convert vm info to json string failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_newVMMetaDataPath = m_metaRepoPath + VIRT_PLUGIN_LIVE_VM_INFO;
    if (Utils::SaveToFileWithRetry(m_metaRepoHandler, m_newVMMetaDataPath, vmInfoStr) != SUCCESS) {
        ERRLOG("Save vm info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    INFOLOG("Save vm info success. VM name: %s, %s", vmInfo.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

int LivemountJob::SubTaskClean()
{
    int result = SUCCESS;
    if (m_dataRepoHandler.get() != nullptr) {
        if (m_dataRepoHandler->Close() != SUCCESS) {
            ERRLOG("Close data repo failed, %s", m_taskInfo.c_str());
        }
    }
    return result;
}

int LivemountJob::PowerOnMachine()
{
    INFOLOG("Enter");
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_liveMountPara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_liveMountPara->extendInfo).c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }
    if (!jobAdvancePara.isMember("config") && !jobAdvancePara["config"].isMember("power_on")) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_liveMountPara->extendInfo).c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }
    m_poweron = jobAdvancePara["config"]["power_on"].asBool();
    if (!m_poweron) {
        INFOLOG("No need to power on, %s", m_taskInfo.c_str());
        return SUCCESS;
    }
    VMInfo poweronVm {};
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, m_newVMMetaDataPath, poweronVm) != SUCCESS) {
        ERRLOG("Load file %s to struct failed, %s", m_newVMMetaDataPath.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    if (m_protectEngine->PowerOnMachine(poweronVm) != SUCCESS) {
        ERRLOG("Power on vm(%s, %s)  failed, %s", poweronVm.m_name.c_str(),
            poweronVm.m_uuid.c_str(), m_taskInfo.c_str());
        return SUCCESS;
    }
    INFOLOG("Power on machine success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int LivemountJob::SubJobPowerOnMachine()
{
    INFOLOG("Enter");
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK);
    int iRet = PowerOnMachine();
    ReportTaskLabel();
    return iRet;
}

// --------------------------------------- Generate sub task-----------------------------------------------------------
void LivemountJob::SetGenerateJobStateMachine()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT)] =
        std::bind(&LivemountJob::GenerateJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PREHOOK)] =
        std::bind(&LivemountJob::GenerateSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_EXEC_JOB)] =
        std::bind(&LivemountJob::CreateSubTasks, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PUT_JOB_TO_FRAME)] =
        std::bind(&LivemountJob::PutSubTasksToFrame, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_POSTHOOK)] =
        std::bind(&LivemountJob::GenerateSubJobPostHook, this);
}

int LivemountJob::GenerateJobInit()
{
    if (!CommonInfoInit()) {
        ERRLOG("Init base info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 存在主任务状态文件 则表明已经执行过当前任务了，直接返回成功
    if (CheckMainTaskStatusFileExist()) {
        INFOLOG("The main task has been executed.skip gen sub job. %s", m_taskInfo.c_str());
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_NONE);
        return SUCCESS;
    }
    m_jobId = m_liveMountPara->jobId;
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_PREHOOK);
    return SUCCESS;
}

int LivemountJob::CreateSubTasks()
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
        ERRLOG("SubJob extend info fomat error, %s", m_taskInfo.c_str());
        return FAILED;
    }
    subJob.__set_jobInfo(infoStr);
    m_execSubs.push_back(subJob);
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_PUT_JOB_TO_FRAME);
    return SUCCESS;
}

int LivemountJob::PutSubTasksToFrame()
{
    ActionResult result {};
    if (!AddNewJobWithRetry(m_execSubs)) {
        ERRLOG("Add new job fail, %s", m_taskInfo.c_str());
        return FAILED;
    }
    SubJobDetails subJobDetails {};
    subJobDetails.__set_jobId(m_jobId);
    subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    std::string description = "Generate sub task for virtualization restore task successfully";
    LogDetail logDetail {};
    std::vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(result, subJobDetails);
    if (result.code != SUCCESS) {
        WARNLOG("Report job detail fail, %s", m_taskInfo.c_str());
    }
    auto ret = GenMainTaskStatusToFile();
    if (ret != SUCCESS) {
        WARNLOG("Failed to save main task status to file, %s", m_taskInfo.c_str());
    }
    INFOLOG("Finish to generate sub job, %s", m_taskInfo.c_str());

    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_POSTHOOK);
    return SUCCESS;
}

int LivemountJob::GenMainTaskStatusToFile()
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

bool LivemountJob::CheckMainTaskStatusFileExist()
{
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("CacheRepoHandler ptr null failed, %s", m_taskInfo.c_str());
        return false;
    }
    return m_cacheRepoHandler->Exists(mainTaskStatusInfoFile);
}

int LivemountJob::PrerequisitePreHook()
{
    INFOLOG("Enter");
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.nextState = static_cast<int>(State::STATE_PRE_CHECK_BEFORE_RECOVER);
    para.postHookState = static_cast<int>(State::STATE_PRE_POSTHOOK);
    return ExecHook(para);
}

int LivemountJob::PrerequisitePostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int LivemountJob::GenerateSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.nextState = static_cast<int>(State::STATE_GEN_CREATE_EXEC_JOB);
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.postHookState = static_cast<int>(State::STATE_GEN_POSTHOOK);
    return ExecHook(para);
}

int LivemountJob::GenerateSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int LivemountJob::ExecuteSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_EXECJOB_INITIALIZE);
    para.postHookState = static_cast<int>(State::STATE_EXECJOB_POSTHOOK);
    return ExecHook(para);
}

int LivemountJob::ExecuteSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}
}