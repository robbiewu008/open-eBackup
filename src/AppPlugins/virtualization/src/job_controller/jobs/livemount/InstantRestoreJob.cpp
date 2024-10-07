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
#include "curl_http/CodeConvert.h"
#include "common/Structs.h"
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/utils/Utils.h"
#include "config_reader/ConfigIniReader.h"
#include "ClientInvoke.h"
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "InstantRestoreJob.h"

namespace {
const std::string MODULE = "INSTANT_RESTORE";
const std::string INSTANT_JOB_INDEX = "1";
const std::string CLEAN_ORIGIN_VM = "1";
}

namespace VirtPlugin {
EXTER_ATTACK int InstantRestoreJob::PrerequisiteJob()
{
    int ret = PreInstantJobInner();
    if (ret != SUCCESS) {
        ReportTaskLabel();
    }
    ReportJobResult(ret, "PrerequisiteJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
}

int InstantRestoreJob::PreInstantJobInner()
{
    DBGLOG("Begin to exeute restore requisite job, %s", m_taskInfo.c_str());
    PreInitInstantStateHandles();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_INIT);
    return RunStateMachine();
}

void InstantRestoreJob::PreInitInstantStateHandles()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_INIT)] =
        std::bind(&InstantRestoreJob::PrerequisiteJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_PREHOOK)] =
        std::bind(&InstantRestoreJob::PrerequisitePreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_CHECK_BEFORE_RECOVER)] =
        std::bind(&InstantRestoreJob::CheckBeforeMount, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_POSTHOOK)] =
        std::bind(&InstantRestoreJob::PrerequisitePostHook, this);
}

int InstantRestoreJob::CheckBeforeMount()
{
    INFOLOG("Enter");
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_POSTHOOK);
    if (m_protectEngine->CheckBeforeMount() != SUCCESS) {
        ERRLOG("Check before recover failed, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    return SUCCESS;
}

EXTER_ATTACK int InstantRestoreJob::GenerateSubJob()
{
    SetGenerateJobStateMachine();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

void InstantRestoreJob::SetGenerateJobStateMachine()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT)] =
        std::bind(&InstantRestoreJob::GenerateJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PREHOOK)] =
        std::bind(&InstantRestoreJob::GenerateSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_EXEC_JOB)] =
        std::bind(&InstantRestoreJob::CreateSubTasks, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PUT_JOB_TO_FRAME)] =
        std::bind(&InstantRestoreJob::PutSubTasksToFrame, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_POSTHOOK)] =
        std::bind(&InstantRestoreJob::GenerateSubJobPostHook, this);
}

int InstantRestoreJob::GenerateJobInit()
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
    m_jobId = m_restorePara->jobId;
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_PREHOOK);
    return SUCCESS;
}

int InstantRestoreJob::GenerateSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.nextState = static_cast<int>(State::STATE_GEN_CREATE_EXEC_JOB);
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.postHookState = static_cast<int>(State::STATE_GEN_POSTHOOK);
    return ExecHook(para);
}

int InstantRestoreJob::CreateSubTasks()
{
    SubJob subJob {};
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName(BUSINESS_SUB_JOB_NAME_PREFIX + INSTANT_JOB_INDEX);
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

int InstantRestoreJob::PutSubTasksToFrame()
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

EXTER_ATTACK int InstantRestoreJob::ExecuteSubJob()
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

int InstantRestoreJob::ExecuteSubJobInner()
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

void InstantRestoreJob::SubJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK)] =
        std::bind(&InstantRestoreJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_INITIALIZE)] =
        std::bind(&InstantRestoreJob::SubTaskInitialize, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME)] =
        std::bind(&InstantRestoreJob::SubTaskExecute, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POWERON_MACHINE)] =
        std::bind(&InstantRestoreJob::SubJobPowerOnMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_MIGRATE_VOLUME)] =
        std::bind(&InstantRestoreJob::SubJobMigrateVolume, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK)] =
        std::bind(&InstantRestoreJob::ExecuteSubJobPostHook, this);
}

int InstantRestoreJob::SubJobMigrateVolume()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK);
    if (m_protectEngine == nullptr) {
        ERRLOG("SubJobMigrateVolume m_protectEngine nullptr, %s", m_taskInfo.c_str());
        return FAILED;
    }
    int iRet = m_protectEngine->MigrateLiveVolume(m_newVm);
    ReportTaskLabel();
    return iRet;
}

int InstantRestoreJob::SubJobPowerOnMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_MIGRATE_VOLUME);
    int iRet = PowerOnMachine();
    ReportTaskLabel();
    return iRet;
}

int InstantRestoreJob::PowerOnMachine()
{
    INFOLOG("Enter");
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }
    if (!jobAdvancePara.isMember("powerState")) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }
    std::size_t* startPos = nullptr;
    int base = 10;
    m_poweron = bool(std::stoi(jobAdvancePara["powerState"].asString(), startPos, base));
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

int InstantRestoreJob::SubTaskInitialize()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME);
    DBGLOG("extendInfo %s extendInfo, copy %s copy", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(),
        WIPE_SENSITIVE(m_restorePara->copies[0].extendInfo).c_str());
    
    if (!LoadMetaData()) {
        ERRLOG("Load meta data failed, %s", m_taskInfo.c_str());
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

bool InstantRestoreJob::LoadMetaData()
{
    if (!LoadVmMetaData()) {
        return false;
    }
    return true;
}

bool InstantRestoreJob::LoadLiveMetaData()
{
    std::string vmmetaDataPath = m_metaRepoPath + VIRT_PLUGIN_LIVE_VM_INFO;
    if (!m_metaRepoHandler->Exists(vmmetaDataPath)) {
        WARNLOG("No need to clean %s, %s", vmmetaDataPath.c_str(), m_taskInfo.c_str());
        return true;
    }
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmmetaDataPath, m_newVm) != SUCCESS) {
        ERRLOG("Failed to read file %s, %s", vmmetaDataPath.c_str(), m_taskInfo.c_str());
        return false;
    }
    return true;
}

int InstantRestoreJob::SubTaskExecute()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POWERON_MACHINE);
    if (m_protectEngine == nullptr) {
        ERRLOG("ProtectEngine ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    int iRet = 0;
    VMInfo newVm;
    iRet = m_protectEngine->CreateLiveMount(m_copyVm, newVm);
    if (iRet != SUCCESS) {
        m_protectEngine->CancelLiveMount(newVm);
        ERRLOG("CreateLiveMount failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_newVm = newVm;
    iRet = SaveVMInfo(newVm);
    if (iRet != SUCCESS) {
        ERRLOG("SaveVMInfo failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    ReportTaskLabel();
    return iRet;
}

int InstantRestoreJob::SaveVMInfo(const VMInfo &vmInfo)
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

EXTER_ATTACK int InstantRestoreJob::PostJob()
{
    int ret = PostJobInner();
    ReportJobResult(ret, "PostJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int InstantRestoreJob::PostJobInner()
{
    if (!CommonInfoInit()) {
        ERRLOG("Post job common init failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    DBGLOG("Begin to run post job, %s", m_taskInfo.c_str());
    PostJobStateInit();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_PREHOOK);
    return RunStateMachine();
}

int InstantRestoreJob::PostJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_PREHOOK)] =
        std::bind(&InstantRestoreJob::PostJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_CLEAN_RESOURCE)] =
        std::bind(&InstantRestoreJob::PostClean, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_DELETE_MACHINE)] =
            std::bind(&InstantRestoreJob::DeleteLiveMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK)] =
        std::bind(&InstantRestoreJob::PostJobPostHook, this);
    return SUCCESS;
}

int InstantRestoreJob::DeleteLiveMachine() // TO DO: 卸载挂载/失败清理
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK);
    m_newVMMetaDataPath = m_metaRepoPath + VIRT_PLUGIN_LIVE_VM_INFO;
    if (m_protectEngine == nullptr) {
        ERRLOG("DeleteLiveMachine m_protectEngine ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (!LoadLiveMetaData()) {
        ERRLOG("LoadLiveMetaData failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_protectEngine->DeleteMachine(m_newVm) != SUCCESS) {
        ERRLOG("Delete machine %s failed, %s", m_newVm.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }
    if (m_protectEngine->CancelLiveMount(m_newVm) != SUCCESS) {
        ERRLOG("Cancel tmp mount datastore of machine %s failed, %s",
            m_newVm.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }
    return SUCCESS;
}

int InstantRestoreJob::PostClean()
{
    if (m_cacheRepoHandler == nullptr || m_protectEngine == nullptr) {
        ERRLOG("CacheRepoHandler or protectEngine ptr null failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_cacheRepoHandler->Exists(m_newVMMetaDataPath)) {
        m_cacheRepoHandler->Remove(m_newVMMetaDataPath);
    }
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(),
            m_taskInfo.c_str());
        return FAILED;
    }
    std::string cleanFlag = jobAdvancePara.isMember("cleanOriginVM"
        ) ? jobAdvancePara["cleanOriginVM"].asString() : "0";
    if (cleanFlag == CLEAN_ORIGIN_VM) {
        if (!LoadMetaData()) {
            ERRLOG("LoadLiveMetaData failed, %s", m_taskInfo.c_str());
            return FAILED;
        }
        if (m_protectEngine->DeleteMachine(m_copyVm) != SUCCESS) {
            ERRLOG("DeleteMachine(name:%s, uuid:%s) failed, %s", m_copyVm.m_name.c_str(),
                m_copyVm.m_uuid.c_str(), m_taskInfo.c_str());
        }
    }
    if (!LoadLiveMetaData()) {
        ERRLOG("LoadLiveMetaData failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_protectEngine->CancelLiveMount(m_newVm) != SUCCESS) {
        ERRLOG("Cancel tmp mount datastore of machine %s failed, %s",
            m_newVm.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK);
    return SUCCESS;
}

int InstantRestoreJob::PostJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::POST_JOB;
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        para.jobExecRet = SUCCESS;
        para.nextState = static_cast<int>(State::STATE_POST_CLEAN_RESOURCE);
    } else {
        para.jobExecRet = FAILED;
        para.nextState = static_cast<int>(State::STATE_POST_DELETE_MACHINE);
    }
    para.postHookState = static_cast<int>(State::STATE_POST_POSTHOOK);
    int ret = ExecHook(para);
    return ret;
}

int InstantRestoreJob::PostJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::POST_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        para.jobExecRet = SUCCESS;
    } else {
        para.jobExecRet = FAILED;
    }
    para.nextState = static_cast<int>(State::STATE_NONE);
    m_nextState = static_cast<int>(State::STATE_NONE);
    return SUCCESS;
}
}
