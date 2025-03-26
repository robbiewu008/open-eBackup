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
#include "RestoreJob.h"
#include <vector>
#include "common/Constants.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "config_reader/ConfigIniReader.h"
#include "ClientInvoke.h"
#include "job_controller/factory/VirtualizationJobFactory.h"

namespace {
const std::string MODULE = "RESTORE";
const std::string NEW_VM_INFO_FILE = VirtPlugin::VIRT_PLUGIN_CACHE_ROOT + "new_vm.info";
const std::string NEW_VM_SNAPSHOT_INFO_FILE = "/new_vm_snapshot.info";
const std::string NEW_VM_RESTORE_SNAPSHOT_NAME = "restore_virtual_vm_snapshot";
using Defer = std::shared_ptr<void>;
constexpr int IO_TIME_OUT = 5; // ms
const std::string MAX_BACKUP_THREADS = "MAX_BACKUP_THREADS";
const std::string VOLUME_DATA_PROCESS = "VOLUME_DATA_PROCESS";
constexpr uint64_t DEFAULT_BACKUP_THREADS = 1;
constexpr uint64_t DEFAULT_SEGMENT_THRESHOLD = 60 * 1024 * 1024 * 1024ULL; // 默认卷分段阈值为60Gb
constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024;
constexpr uint32_t RETRY_INTERVAL_SECOND = 3;
constexpr uint32_t ARCHIVERESTORE_IOJOB_INTERVAL_SECOND = 1;
const std::string ALARM_CODE_FAILED_DELETE_MACHINE = "0x6403400008";
const std::string ALARM_CODE_FAILED_DELETE_VOLUME = "0x6403400009";
}

namespace VirtPlugin {
EXTER_ATTACK int RestoreJob::PrerequisiteJob()
{
    int ret = PrerequisiteJobInner();
    if (ret != SUCCESS) {
        ReportTaskLabel();
    }
    ReportJobResult(ret, "PrerequisiteJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int RestoreJob::PrerequisiteJobInner()
{
    DBGLOG("Begin to exeute restore requisite job, %s", m_taskInfo.c_str());
    PreInitStateHandles();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_INIT);
    return RunStateMachine();
}

void RestoreJob::PreInitStateHandles()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_INIT)] =
        std::bind(&RestoreJob::PrerequisiteJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_PREHOOK)] =
        std::bind(&RestoreJob::PrerequisitePreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_CHECK_BEFORE_RECOVER)] =
        std::bind(&RestoreJob::CheckBeforeRecover, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_GEN_INITIAL_VOL_LIST)] =
        std::bind(&RestoreJob::GenInitialRestoreVolList, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_GEN_FINAL_VOL_LIST)] =
        std::bind(&RestoreJob::GenFinalRestoreVolList, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_POWER_OFF_MACHINE)] =
        std::bind(&RestoreJob::PowerOffMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_DETACH_VOL)] =
        std::bind(&RestoreJob::DetachVolume, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_PRE_POSTHOOK)] =
        std::bind(&RestoreJob::PrerequisitePostHook, this);
}

int RestoreJob::CheckBeforeRecover()
{
    if (m_protectEngine->CheckBeforeRecover(m_copyVm) != SUCCESS) {
        ERRLOG("Check before recover failed, %s", m_taskInfo.c_str());
        ReportJobDetailWithErrorParams();
        return FAILED;
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_GEN_INITIAL_VOL_LIST);
    return SUCCESS;
}

bool RestoreJob::InitRepo() const
{
    std::string restoreJobCachePath = m_cacheRepoPath + VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT;
    if (!m_cacheRepoHandler->Exists(restoreJobCachePath)) {
        DBGLOG("Creating restore cache directory: %s, %s", restoreJobCachePath.c_str(), m_taskInfo.c_str());
        int res = Utils::RetryOpWithT<int>(std::bind(&RepositoryHandler::CreateDirectory, m_cacheRepoHandler,
            restoreJobCachePath), true, "CreateDirectory");
        if (res != SUCCESS) {
            ERRLOG("Create restore cache directory %s failed, %s", restoreJobCachePath.c_str(), m_taskInfo.c_str());
            return false;
        }
    }
    return true;
}

bool RestoreJob::CommonInfoInit()
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("JobCommonInfo is null, %s", m_taskInfo.c_str());
        return false;
    }
    m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    if (m_restorePara == nullptr) {
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
    if (!InitRestoreParams()) {
        ERRLOG("Init restore param failed, %s", m_taskInfo.c_str());
        return false;
    }
    m_newVMMetaDataPath = m_cacheRepoPath + NEW_VM_INFO_FILE;
    m_volPairPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    return true;
}

int RestoreJob::PrerequisiteJobInit()
{
    if (!CommonInfoInit()) {
        ERRLOG("Init common info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 存在主任务状态文件则表明已经执行过当前任务了，直接返回成功
    if (CheckMainTaskStatusFileExist()) {
        INFOLOG("The main task has been executed.skip prerequisite job. %s", m_taskInfo.c_str());
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_NONE);
        return SUCCESS;
    }
    if (CheckForciblyRestoreInvalidCopies() != SUCCESS) {
        ERRLOG("Forcibly restore invalid copies failed.");
        return FAILED;
    }
    if (!LoadMetaData()) {
        ERRLOG("Load meta data failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::vector<std::string> reportArgs;
    reportArgs.push_back(m_restorePara->copies[0].id.c_str());
    ReportJobDetailsParam reportParam = {
        "virtual_plugin_select_copy_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0 };
    ReportJobDetailWithLabel(reportParam, reportArgs);
    DBGLOG("PrerequisiteJobInit success, %s", m_taskInfo.c_str());
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_PREHOOK);
    return SUCCESS;
}

bool RestoreJob::InitRestoreParams()
{
    if (m_restorePara->restoreSubObjects.empty()) {
        ERRLOG("No restore volumes from req, %s", m_taskInfo.c_str());
        return false;
    }
    // 恢复虚拟机信息
    bool ret = Module::JsonHelper::JsonStringToStruct(m_restorePara->targetObject.extendInfo, m_restoreVm);
    if (!ret) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->targetObject.extendInfo).c_str(),
            m_taskInfo.c_str());
        return false;
    }
    m_restoreVm.m_uuid = m_restorePara->targetObject.id;
    m_restoreVm.m_name = m_restorePara->targetObject.name;
    Json::Value targetObjInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->targetObject.extendInfo, targetObjInfo)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->targetObject.extendInfo).c_str(),
            m_taskInfo.c_str());
        return false;
    }
    std::size_t* startPos = nullptr;
    int base = 10;
    m_poweron = bool(std::stoi(targetObjInfo["powerState"].asString(), startPos, base));
    Json::Value jobAdvancePara;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobAdvancePara)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(), m_taskInfo.c_str());
        return false;
    }
    m_RestoreLevel = RestoreLevel(std::stoi(jobAdvancePara["restoreLevel"].asString(), startPos, base));
    return true;
}

/**
 * @brief 卷恢复可能需要创建新卷
 *
 * @return int
 */
int RestoreJob::GenFinalRestoreVolList()
{
    if (m_cacheRepoHandler->Exists(m_volPairPath)) {
        INFOLOG("Already exist volume pair, %s", m_taskInfo.c_str());
        if (Utils::LoadFileToStruct(m_cacheRepoHandler, m_volPairPath, m_finalRestoreVolPair) != SUCCESS) {
            ERRLOG("Load %s failed, %s", m_volPairPath.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_POWER_OFF_MACHINE);
        return SUCCESS;
    }
    for (auto &it : m_initialRestoreVolsMap) { // 备份卷元数据组合成的uuid, VolInfo 映射表
        std::string backupVolUUID = it.first;
        int volIdx = -1;
        for (int index = 0; index < m_restorePara->restoreSubObjects.size(); index++) {
            if (m_restorePara->restoreSubObjects[index].id == backupVolUUID) {
                volIdx = index;
                break;
            }
        }
        if (volIdx == -1) { // 未匹配到
            ERRLOG("Not find volume for %s, %s", backupVolUUID.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
        if (m_protectEngine->GenVolPair(
            m_restoreVm, it.second, m_restorePara->restoreSubObjects[volIdx], m_finalRestoreVolPair) != SUCCESS) {
                ERRLOG("Gen vol pair failed, %s", m_taskInfo.c_str());
            ReportTaskLabel();
            return FAILED;
        }
    }
    std::string volPairstr = "";
    if (!Module::JsonHelper::StructToJsonString(m_finalRestoreVolPair, volPairstr)) {
        ERRLOG("Convert vol pair failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, m_volPairPath, volPairstr) != SUCCESS) {
        ERRLOG("Save %s failed, %s", WIPE_SENSITIVE(volPairstr).c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_POWER_OFF_MACHINE);
    return SUCCESS;
}

int RestoreJob::DetachVolume()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_POSTHOOK);
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        INFOLOG("Vm restore no need detach volume.");
        return SUCCESS;
    }
    for (auto &volPair : m_finalRestoreVolPair.m_volPairList) {
        if (m_protectEngine->DetachVolume(volPair.m_targetVol) != SUCCESS) {
            ERRLOG("Detach volume %s failed, %s", volPair.m_targetVol.m_uuid.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
    }
    ReportTaskLabel();
    INFOLOG("Detach volumes success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

EXTER_ATTACK int RestoreJob::GenerateSubJob()
{
    SetGenerateJobStateMachine();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int RestoreJob::SubJobPowerOnMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_POSTHOOK);
    return PowerOnMachine();
}

int RestoreJob::PostJobPowerOnMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_CLEAN_RESOURCE);
    return PowerOnMachine();
}

int RestoreJob::PowerOnMachine()
{
    if (!m_poweron) {
        INFOLOG("No need to power on vm, %s", m_taskInfo.c_str());
        return SUCCESS;
    }
    m_protectEngine->SetJobResult(m_jobResult);
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        VMInfo poweronVm {};
        if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_newVMMetaDataPath, poweronVm) != SUCCESS) {
            ERRLOG("Load file %s to struct failed, %s", m_newVMMetaDataPath.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
        if (m_protectEngine->PowerOnMachine(poweronVm) != SUCCESS) {
            ERRLOG("Power on vm(%s, %s)  failed, %s", poweronVm.m_name.c_str(),
                poweronVm.m_uuid.c_str(), m_taskInfo.c_str());
        }
        return SUCCESS;
    }
    if (m_protectEngine->PowerOnMachine(m_restoreVm) != SUCCESS) {
        ERRLOG("Power on vm(%s) failed, %s", m_restoreVm.m_uuid.c_str(), m_taskInfo.c_str());
        return SUCCESS;
    }
    ReportTaskLabel();
    INFOLOG("Power on machine success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int RestoreJob::SubJobAttachVolume()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_POWERON_MACHINE);
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_volPairPath, m_finalRestoreVolPair) != SUCCESS) {
        ERRLOG("Load vol_match.info to struct failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    // 将所有新建的卷排在容器末尾，先挂载已有卷，避免盘符占用
    VolMatchPairInfo restoreVolPair;
    std::vector<VolPair> newVolPairList;
    for (const VolPair &volPair : m_finalRestoreVolPair.m_volPairList) {
        if (volPair.m_targetVol.m_newCreate) {
            newVolPairList.push_back(volPair);
        } else {
            restoreVolPair.m_volPairList.push_back(volPair);
        }
    }
    restoreVolPair.m_volPairList.insert(restoreVolPair.m_volPairList.end(),
        newVolPairList.begin(), newVolPairList.end());
    m_protectEngine->ClearLabel();
    for (const auto &volPair : restoreVolPair.m_volPairList) {
        if (m_protectEngine->AttachVolume(volPair.m_targetVol) != SUCCESS) {
            ERRLOG("Attach vol failed. volume uuid=%s, %s", volPair.m_targetVol.m_uuid.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
        DBGLOG("Attach volume %s success,%s", volPair.m_targetVol.m_uuid.c_str(), m_taskInfo.c_str());
    }
    ReportTaskLabel();
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        if (m_protectEngine->RestoreVolMetadata(m_finalRestoreVolPair, m_copyVm) != SUCCESS) {
            ERRLOG("Restore volume metadata failed, %s", m_taskInfo.c_str());
            ReportTaskLabel();
            return FAILED;
        }
    }
    INFOLOG("Attach volumes success, %s", m_taskInfo.c_str());
    return SUCCESS;
}

int RestoreJob::PostJobAttachVolume()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POWERON_MACHINE);
    if (!m_cacheRepoHandler->Exists(m_volPairPath)) { // 还没有创建卷匹配文件，表示还没有到卸载，不需要挂载上去
        return SUCCESS;
    }
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_volPairPath, m_finalRestoreVolPair) != SUCCESS) {
        ERRLOG("Load vol_match.info to struct failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    VolMatchPairInfo restoreVolPair;
    for (const VolPair &volPair : m_finalRestoreVolPair.m_volPairList) {
        if (volPair.m_targetVol.m_newCreate) {
            restoreVolPair.m_volPairList.push_back(volPair);
        } else {
            restoreVolPair.m_volPairList.insert(restoreVolPair.m_volPairList.begin(), volPair);
        }
    }
    for (auto &volPair : restoreVolPair.m_volPairList) {
        if (m_jobResult != AppProtect::JobResult::type::SUCCESS && volPair.m_targetVol.m_newCreate) {
            continue; // 失败场景下新创卷不能挂载到虚拟机
        }
        if (m_protectEngine->AttachVolume(volPair.m_targetVol) != SUCCESS) {
            ERRLOG("Attach vol failed. volume uuid=%s, %s", volPair.m_targetVol.m_uuid.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
    }
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        if (m_protectEngine->RestoreVolMetadata(m_finalRestoreVolPair, m_copyVm) != SUCCESS) {
            ERRLOG("Restore volume metadata failed, %s", m_taskInfo.c_str());
            ReportTaskLabel();
            return FAILED;
        }
    }
    return SUCCESS;
}

int RestoreJob::DeleteMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_DELETE_VOLUMES);
    if (!m_cacheRepoHandler->Exists(m_newVMMetaDataPath)) {
        INFOLOG("Not exist virtual machine, path: %s, %s", m_newVMMetaDataPath.c_str(), m_taskInfo.c_str());
        return SUCCESS;
    }
    VMInfo newVm {};
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_newVMMetaDataPath, newVm) != SUCCESS) {
        ERRLOG("Load file %s to struct failed, %s", m_newVMMetaDataPath.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    if (m_protectEngine->DeleteMachine(newVm) != SUCCESS) {
        ERRLOG("Delete machine %s failed, %s", newVm.m_name.c_str(), m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }
    return SUCCESS;
}

int RestoreJob::DeleteVolumes()
{
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_CLEAN_RESOURCE);
    } else {
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_ATTACH_VOLUMES);
    }
    if (!m_cacheRepoHandler->Exists(m_volPairPath)) {
        INFOLOG("Not exist vol pair, path=%s, %s", m_volPairPath.c_str(), m_taskInfo.c_str());
        return SUCCESS;
    }
    std::string newVolMetaData = "";
    int res = Utils::RetryOpWithT<int>(std::bind(&Utils::ReadFile, m_cacheRepoHandler, m_volPairPath,
        std::ref(newVolMetaData)), SUCCESS, "ReadFile");
    if (res != SUCCESS) {
        ERRLOG("Failed to read file %s, %s", m_volPairPath.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(newVolMetaData, m_finalRestoreVolPair)) {
        ERRLOG("Convert %s to json failed, %s", newVolMetaData.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    int ret = SUCCESS;
    for (auto &volPair : m_finalRestoreVolPair.m_volPairList) {
        if (!volPair.m_targetVol.m_newCreate) {
            continue;
        }
        if (m_protectEngine->DetachVolume(volPair.m_targetVol) != SUCCESS ||
            m_protectEngine->DeleteVolume(volPair.m_targetVol) != SUCCESS) {
            ERRLOG("Failed to delete volume %s, %s", volPair.m_targetVol.m_name.c_str(), m_taskInfo.c_str());
            ReportTaskLabel();
            ret = FAILED;
        }
        INFOLOG("Success to delete vol %s, %s", volPair.m_targetVol.m_name.c_str(), m_taskInfo.c_str());
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_ATTACH_VOLUMES);
    return ret;
}

int RestoreJob::PostClean()
{
    if (m_cacheRepoHandler->Exists(m_newVMMetaDataPath)) {
        m_cacheRepoHandler->Remove(m_newVMMetaDataPath);
    }
    if (m_cacheRepoHandler->Exists(m_volPairPath)) {
        m_cacheRepoHandler->Remove(m_volPairPath);
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK);
    return SUCCESS;
}

EXTER_ATTACK int RestoreJob::PostJob()
{
    int ret = PostJobInner();
    ReportJobResult(ret, "PostJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int RestoreJob::PostJobInner()
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

int RestoreJob::PostJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_PREHOOK)] =
        std::bind(&RestoreJob::PostJobPreHook, this);
    if (m_jobResult != AppProtect::JobResult::type::SUCCESS) {
        if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
            m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_DELETE_MACHINE)] =
                std::bind(&RestoreJob::DeleteMachine, this);
            m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_DELETE_VOLUMES)] =
                std::bind(&RestoreJob::DeleteVolumes, this); // 删除新创卷挂载
        } else {
            m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_DELETE_VOLUMES)] =
                std::bind(&RestoreJob::DeleteVolumes, this); // 删除新创卷挂载
            m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_ATTACH_VOLUMES)] =
                std::bind(&RestoreJob::PostJobAttachVolume, this); // 挂载原卷到虚拟机
            m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_POWERON_MACHINE)] =
                std::bind(&RestoreJob::PostJobPowerOnMachine, this);
        }
    }
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_CLEAN_RESOURCE)] =
        std::bind(&RestoreJob::PostClean, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_POST_POSTHOOK)] =
        std::bind(&RestoreJob::PostJobPostHook, this);
    return SUCCESS;
}
/**
 * @brief 初始化应用引擎实例及存储仓库handler以及存储仓库路径
 *
 * @return true: 成功, false: 失败
 */
bool RestoreJob::InitHandlers()
{
    if (m_restorePara->copies.empty()) {
        ERRLOG("No copy data, %s", m_taskInfo.c_str());
        return false;
    }
    // 保护对象Handler
    if (InitProtectEngineHandler(JobType::RESTORE) != SUCCESS) {
        ERRLOG("Initialize protect engine handler failed, %s", m_taskInfo.c_str());
        return false;
    }
    INFOLOG("Copy name is %s, copy id %s, %s", m_restorePara->copies[0].name.c_str(),
        m_restorePara->copies[0].id.c_str(), m_taskInfo.c_str());
    std::vector<StorageRepository> repoList = m_restorePara->copies[0].repositories;
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
                        !DoInitHandlers(repo, m_dataRepoHandler, m_dataRepoPath)
#ifdef WIN32
&& !m_isArchiveRestore
#endif
        ) {
            ERRLOG("Init data repo handler failed, %s", m_taskInfo.c_str());
            return false;
        } else if (repo.repositoryType == RepositoryDataType::META_REPOSITORY &&
                        !DoInitHandlers(repo, m_metaRepoHandler, m_metaRepoPath)
#ifdef WIN32
&& !m_isArchiveRestore
#endif
        ) {
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

/**
 * @brief 读取卷元数据信息
 *
 * @return true
 * @return false
 */
bool RestoreJob::LoadVolMetaData()
{
    VolInfo volInfo;
    for (const auto &vol : m_restorePara->restoreSubObjects) {
        std::string volmetaDataPath = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_META_DIR + vol.id + ".ovf";
        if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, volmetaDataPath, volInfo) != SUCCESS) {
            ERRLOG("Failed to load file %s, %s", volmetaDataPath.c_str(), m_taskInfo.c_str());
            return false;
        }
        m_backupedVolList.push_back(volInfo);
    }
    m_copyVm.m_volList = m_backupedVolList;
    return true;
}

/**
 * @brief 读取虚拟机元数据
 *
 * @return true
 * @return false
 */
bool RestoreJob::LoadVmMetaData()
{
    std::string vmmetaDataPath = m_metaRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, vmmetaDataPath, m_copyVm) != SUCCESS) {
        ERRLOG("Failed to read file %s, %s", vmmetaDataPath.c_str(), m_taskInfo.c_str());
        return false;
    }
    return true;
}

/**
 * @brief 获取虚拟机元数据信息
 * @param vmMetaData
 * @return true: 成功, false: 失败
 */
bool RestoreJob::LoadMetaData()
{
    if (!LoadVmMetaData() || !LoadVolMetaData()) {
        return false;
    }
    return true;
}

/**
 * @brief 强制恢复无效的副本检查
 * @param
 * @return int
 */
int RestoreJob::CheckForciblyRestoreInvalidCopies()
{
    Json::Value jobExtendInfo;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_restorePara->extendInfo, jobExtendInfo)) {
        ERRLOG("Convert restore extend info failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str());
        return FAILED;
    }
    if (jobExtendInfo.isMember("force_recovery") &&
        jobExtendInfo["force_recovery"].asString() == "true") {
        std::string forceRecoverFlag = Module::ConfigReader::getString("General", "RecoverIgnoreBadBlock");
        INFOLOG("Invalid Copy force recovery and the force recover flag is: %s", forceRecoverFlag.c_str());
        if (forceRecoverFlag != "yes") {
            ReportJobDetailsParam reportParam = {"virtual_plugin_current_copy_is_invalid_label",
                                                 JobLogLevel::TASK_LOG_ERROR,
                                                 SubJobStatus::RUNNING, 0, 0};
            std::vector <std::string> args;
            ReportJobDetailWithLabel(reportParam, args);
            return FAILED;
        }
        ReportJobDetailsParam reportParam = {"virtual_plugin_forcibly_restore_invalid_copies_label",
                                             JobLogLevel::TASK_LOG_WARNING,
                                             SubJobStatus::RUNNING, 0, 0};
        std::vector <std::string> args;
        ReportJobDetailWithLabel(reportParam, args);
    }
    return SUCCESS;
}

/**
 * @brief 根据PM下发的卷列表和副本数据中的卷列表生成初始待恢复卷列表(存储数据结构为map<uuid, VolInfo>)
 *
 * @return true: 成功, false: 失败
 */
int RestoreJob::GenInitialRestoreVolList()
{
    for (auto restoreVol : m_restorePara->restoreSubObjects) {
        std::transform(restoreVol.id.begin(), restoreVol.id.end(), restoreVol.id.begin(), ::tolower);
        bool find = false;
        for (auto backupVol : m_backupedVolList) {
            std::string backupVolUUID = backupVol.m_uuid;
            std::transform(backupVolUUID.begin(), backupVolUUID.end(), backupVolUUID.begin(), ::tolower);
            if (restoreVol.id != backupVolUUID) {
                continue;
            }
            find = true;
            m_initialRestoreVolsMap[backupVolUUID] = backupVol;
            break;
        }
        if (!find) {
            ERRLOG("Do not find match disk with uuid[%s], %s", restoreVol.id.c_str(), m_taskInfo.c_str());
            return FAILED;
        }
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_GEN_FINAL_VOL_LIST);
    return SUCCESS;
}

/**
 * @brief 恢复到原位置(非替换原机)和新位置需要创虚拟机
 *
 * @return int
 */
int RestoreJob::CreateMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_ATTACH_VOLUME);
    if (!LoadVmMetaData() || Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, m_volPairPath,
        m_finalRestoreVolPair) != SUCCESS) {
        ERRLOG("Load meta data failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        return SUCCESS;
    }
    m_protectEngine->ClearLabel();
    // 仅整机恢复均需要创建新机
    if (m_protectEngine->CreateMachine(m_copyVm) != SUCCESS) {
        ERRLOG("Create machine failed, %s", m_taskInfo.c_str());
        ReportTaskLabel(std::vector<std::string>{m_restorePara->targetObject.name});
        m_protectEngine->DeleteMachine(m_copyVm);
        return FAILED;
    }
    if (m_protectEngine->RestoreVolMetadata(m_finalRestoreVolPair, m_copyVm) != SUCCESS) {
        ERRLOG("Restore volume metadata failed, %s", m_taskInfo.c_str());
        ReportTaskLabel();
        return FAILED;
    }

    std::string newVMInfoStr = "";
    m_restoreVm = m_copyVm;
    if (!Module::JsonHelper::StructToJsonString(m_restoreVm, newVMInfoStr)) {
        ERRLOG("Convert VM info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, m_newVMMetaDataPath, newVMInfoStr) != SUCCESS) {
        ERRLOG("Save %s to file failed, %s", WIPE_SENSITIVE(newVMInfoStr).c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int RestoreJob::PowerOffMachine()
{
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_DETACH_VOL);
    if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        INFOLOG("Vm restore no need poweroff machine.");
        return SUCCESS;
    }
    if (m_protectEngine->PowerOffMachine(m_restoreVm) != SUCCESS) {
        ERRLOG("Poweroff machine %s failed, %s", m_restoreVm.m_name.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    ReportTaskLabel();
    DBGLOG("Power off machine(%s) success, %s", m_restoreVm.m_name.c_str(), m_taskInfo.c_str());
    return SUCCESS;
}

/**
 *  @brief 子任务执行入口
 *
 *  @return 错误码：0 成功，非0 失败
 */
EXTER_ATTACK int RestoreJob::ExecuteSubJob()
{
    int ret = ExecuteSubJobInner();
    if (ret != SUCCESS) {
        ReportTaskLabel();
    }
    ReportJobResult(ret, "ExecuteSubJob finish.", m_completedDataSize);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    std::string nodeBalance = Module::ConfigReader::getString(GENERAL_CONF, "DataRepoPathBalance");
    if (nodeBalance == "true" && m_subJobInfo->jobName.find(BUSINESS_SUB_JOB_NAME_PREFIX) != std::string::npos) {
        LoadBalancer::GetInstance()->RemoveNodePath(m_dataRepoPath);
    }
    SetJobToFinish();
    return ret;
}

void RestoreJob::PostSubJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_PREHOOK)] =
        std::bind(&RestoreJob::PostSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_CREATE_MACHINE)] =
        std::bind(&RestoreJob::CreateMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_ATTACH_VOLUME)] =
        std::bind(&RestoreJob::SubJobAttachVolume, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_POWERON_MACHINE)] =
        std::bind(&RestoreJob::SubJobPowerOnMachine, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_POSTHOOK)] =
        std::bind(&RestoreJob::PostSubJobPostHook, this);
}

int RestoreJob::ExecuteSubJobInner()
{
    if (!CommonInfoInit()) {
        ERRLOG("Failed to Init common info, %s", m_taskInfo.c_str());
        return FAILED;
    }
    int ret = SUCCESS;

    if (m_subJobInfo->jobName == "PostSubJob") {
        PostSubJobStateInit();
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_PREHOOK);
        ret = RunStateMachine();
    } else {
        SubJobStateInit();
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK);
        ret = RunStateMachine();
        // 清理资源
        if ((SubTaskClean() != SUCCESS) || (ret != SUCCESS)) {
            ERRLOG("Failed to clean task, ret=%d, %s", ret, m_taskInfo.c_str());
            return FAILED;
        }
    }
    return ret;
}

void RestoreJob::SubJobStateInit()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK)] =
        std::bind(&RestoreJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_INITIALIZE)] =
        std::bind(&RestoreJob::SubTaskInitialize, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_LOAD_DIRTYRANGES)] =
        std::bind(&RestoreJob::SubTaskGetDirtyRanges, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME)] =
        std::bind(&RestoreJob::SubTaskExecute, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK)] =
        std::bind(&RestoreJob::ExecuteSubJobPostHook, this);
}

int RestoreJob::InitTaskScheduler()
{
    TaskPool *tp = TaskPool::GetInstance();
    m_taskScheduler = std::make_unique<TaskScheduler>(*tp);
    if (m_taskScheduler == nullptr) {
        ERRLOG("Failed to create task scheduler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    return SUCCESS;
}

/**
 *  @brief 执行子任务_初始化参数
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::SubTaskInitialize()
{
    // 加载成员变量参数
    if (InitExecJobParams() != SUCCESS) {
        ERRLOG("Failed to create init params, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::vector<std::string> reportArgs;
    reportArgs.push_back(m_originVolInfo.m_uuid.c_str());
    reportArgs.push_back(m_targetVolInfo.m_uuid.c_str());
    ReportTaskLabel(reportArgs);

    // 获取读端和写端handler
    if (CreateIOHandler() != SUCCESS) {
        ERRLOG("Failed to create IO handler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (InitTaskScheduler() != SUCCESS) {
        ERRLOG("Failed to init task scheduler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_LOAD_DIRTYRANGES);
    return SUCCESS;
}

/**
 *  @brief 执行子任务_获取副本的dirtyRanges
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::SubTaskGetDirtyRanges()
{
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    INFOLOG("Volume Size In Bytes: %llu, m_completedSegmentSize: %llu, m_segmentSizeConf: %llu, taskid: %s",
        m_originVolInfo.m_volSizeInBytes, m_completedSegmentSize, m_segmentSizeConf, m_taskInfo.c_str());
    if (m_originVolInfo.m_volSizeInBytes <= (m_completedSegmentSize + m_segmentSizeConf)) {
        DBGLOG("m_needSegment = false");
        startOffset = m_completedSegmentSize;
        endOffset = m_originVolInfo.m_volSizeInBytes;
        m_curSegmentSize = endOffset - startOffset;
        m_needSegment = false;
    } else {
        DBGLOG("m_needSegment = true");
        startOffset = m_completedSegmentSize;
        endOffset = m_completedSegmentSize + m_segmentSizeConf;
        m_curSegmentSize = m_segmentSizeConf;
        m_needSegment = true;
    }
    DBGLOG("StartOffset: %llu, endOffset: %llu, taskid: %s", startOffset, endOffset, m_taskInfo.c_str());
    VolSnapInfo SnapShotObj;
    VolSnapInfo curSnap;
    // 原卷恢复需要获取副本快照信息，异卷恢复直接全量恢复，不需要获取副本快照信息(快照可能不是)
    // 根据m_uuid和m_datastore.m_type判断是否是相同类型上的原卷
    if (m_targetVolInfo.m_uuid == m_originVolInfo.m_uuid &&
        m_targetVolInfo.m_datastore.m_type == m_originVolInfo.m_datastore.m_type && !GetSnapshotInfo(SnapShotObj)) {
        ERRLOG("Failed to get copy vol snapshot info, %s", m_taskInfo.c_str());
        return FAILED;
    }
    m_dirtyRanges.clear();
    if (m_targetVolHandler->GetDirtyRanges(SnapShotObj, curSnap, m_dirtyRanges, startOffset, endOffset) != SUCCESS) {
        ERRLOG("Failed to get dirtyRange, %s", m_taskInfo.c_str());
        return FAILED;
    }

    m_dirtyRanges.Initialize(m_cacheRepoPath, m_originVolInfo.m_uuid, m_cacheRepoHandler);
    if (!m_dirtyRanges.FlushToStorage()) {
        ERRLOG("Save dirty range file failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Finish vol(%s, %s) start offset(%llu) dirty range, %s", m_targetVolInfo.m_name.c_str(),
        m_targetVolInfo.m_uuid.c_str(), startOffset, m_taskInfo.c_str());
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_RESTORE_VOLUME);
    return SUCCESS;
}

int RestoreJob::LoadCurSegBlockBitMap()
{
    uint64_t confSegSize = GetSegementSizeFromConf();
    if (confSegSize <= 0) {
        return FAILED;
    }
    if (m_blockDataBitMap == nullptr) {
        uint64_t stdSegBlockCount = (m_segmentSizeConf + DEFAULT_BLOCK_SIZE - 1) / DEFAULT_BLOCK_SIZE;
        m_blockDataBitMap.reset(new uint8_t[stdSegBlockCount], std::default_delete<uint8_t[]>());
    }

    uint64_t finishedSegmentCount = m_completedSegmentSize / confSegSize;  // 已经完成的段数量
    uint64_t perSegBlockCount = (confSegSize + DIRTY_RANGE_BLOCK_SIZE - 1) / DIRTY_RANGE_BLOCK_SIZE;
    uint64_t curSegBlockStartIndex = finishedSegmentCount * perSegBlockCount;
    uint64_t curSegBlockCount = (m_curSegmentSize + DIRTY_RANGE_BLOCK_SIZE - 1) / DIRTY_RANGE_BLOCK_SIZE ;
    DBGLOG("confSegSize(%llu), finishedSegmentCount(%llu), perSegBlockCount(%llu), %s.", confSegSize,
           finishedSegmentCount, perSegBlockCount, m_taskInfo.c_str());
    std::string metaRepoPath = m_isArchiveRestore ? m_cacheRepoPath : m_metaRepoPath;
    std::shared_ptr<RepositoryHandler> metaRepoHandler = m_isArchiveRestore ? m_cacheRepoHandler : m_metaRepoHandler;
    std::string dirtyRangeBitMapFile = metaRepoPath + VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP +
        m_originVolInfo.m_uuid + "_block_bitmap.info";
    DBGLOG("Bitmapfile(%s), curSegBlockStartIndex(%llu), curSegBlockCount(%llu), %s.", dirtyRangeBitMapFile.c_str(),
        curSegBlockStartIndex, curSegBlockCount, m_taskInfo.c_str());
    if (metaRepoHandler->Open(dirtyRangeBitMapFile, "r") != SUCCESS) {
        ERRLOG("Failed to open %s.", dirtyRangeBitMapFile.c_str());
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) {
        metaRepoHandler->Close();
    });
    int retrytimes = 3;
    while (retrytimes > 0) {
        if (metaRepoHandler->Seek(curSegBlockStartIndex) != SUCCESS) {
            ERRLOG("Seek %llu failed, %s.", curSegBlockStartIndex, m_taskInfo.c_str());
            Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
            retrytimes--;
            continue;
        }
        if (metaRepoHandler->Read(m_blockDataBitMap, curSegBlockCount) == curSegBlockCount) {
            DBGLOG("Read dirty bit map succ, start index(%llu), size(%llu), %s.", curSegBlockStartIndex,
                   curSegBlockCount, m_taskInfo.c_str());
            return SUCCESS;
        }
        ERRLOG("Read dirty range curSegBitMap failed, will retry, %s.", m_taskInfo.c_str());
        Utils::SleepSeconds(RETRY_INTERVAL_SECOND);
        retrytimes--;
    }
    return FAILED;
}

bool RestoreJob::ExecBlockTaskStart(const std::shared_ptr<RestoreIOTask> &task,
    int &tasksCount, int &tasksExecRet)
{
    tasksExecRet = SUCCESS;
    if (tasksCount < m_maxIOThreadNumConf) {
        if (!m_taskScheduler->Put(task)) {
            ERRLOG("Put io task to pool failed, %s", m_taskInfo.c_str());
            return false;
        }
        ++tasksCount;
        return true;
    }
    if (!IOTaskExecResult(tasksCount)) {
        tasksExecRet = FAILED;
    }
    return false;
}

bool RestoreJob::CheckCurBlockHasData(uint64_t offset) const
{
    uint64_t curSegBlockOffset = offset - m_completedSegmentSize;
    uint64_t curBlockIndex = (curSegBlockOffset + DIRTY_RANGE_BLOCK_SIZE - 1) / DIRTY_RANGE_BLOCK_SIZE;
    if (m_blockDataBitMap[curBlockIndex] == '0') { // 0-no data
        return false;
    }
    return true;
}

bool RestoreJob::IfCheckDataBitMap() const
{
    std::string dirtyRangeBitMapFile = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP +
        m_originVolInfo.m_uuid + "_block_bitmap.info";
    return m_metaRepoHandler->Exists(dirtyRangeBitMapFile);
}

bool RestoreJob::IfCheckDataBitMapArchive() const
{
    std::string dirtyRangeBitMapFile = m_cacheRepoPath + VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP +
        m_originVolInfo.m_uuid + "_block_bitmap.info";
    DBGLOG("dirtyRangeBitMapFile path, %s", dirtyRangeBitMapFile.c_str());
    return m_cacheRepoHandler->Exists(dirtyRangeBitMapFile);
}

/**
 *  @brief 执行子任务_对volume进行IO操作
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::SubTaskExecute()
{
    bool dataBitMapCheckFlag =  m_isArchiveRestore ? IfCheckDataBitMapArchive() : IfCheckDataBitMap();
    if (dataBitMapCheckFlag && LoadCurSegBlockBitMap() != SUCCESS) {
        return FAILED;
    }
    int nTaskExecuting = 0;  // 当前尚在执行的任务数
    uint64_t preReportNum = 0;
    DirtyRanges::iterator it = m_dirtyRanges.begin(DIRTY_RANGE_BLOCK_SIZE);
    int taskExecRet = SUCCESS;
    while (taskExecRet != FAILED && !it.End()) {
        if (dataBitMapCheckFlag && !CheckCurBlockHasData(it->Offset())) {
            ++it;
            continue;
        }
        std::shared_ptr<RestoreIOTask> task;
        if (m_isArchiveRestore) {
            ArchiveTaskInitial(it, task);
            m_maxIOThreadNumConf = 1;
        } else {
            task = std::make_shared<RestoreIOTask>(it->Offset(), it->size, m_targetVolHandler, m_dataRepoHandler);
            task->SetIgnoreBadBlockFlag(m_forceRecoverIgnoreBadBlock);
        }
        bool goNext = ExecBlockTaskStart(task, nTaskExecuting, taskExecRet);
        if (goNext) { // goNext用于标记能否继续往调度器中添加任务，如果不能则重新加入进行重试
            ++it;
        }
        if (taskExecRet != SUCCESS) {
            break;
        }
        if (preReportNum != (m_completedDataSize / GB_SIZE)) {
            preReportNum = (m_completedDataSize / GB_SIZE);
            ReportJobSpeed(m_completedDataSize); // 上报数据量
        }
    }
    ExecIOTaskEnd(nTaskExecuting, taskExecRet);
    if (taskExecRet != SUCCESS) {
        ERRLOG("Failed to exec sub task, volume id(%s), %s", m_targetVolInfo.m_uuid.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    Utils::RetryOpWithT<int>(std::bind(&VolumeHandler::Flush, m_targetVolHandler), SUCCESS, "Flush");
    if (m_needSegment) {
        m_completedSegmentSize += m_segmentSizeConf;
        DBGLOG("Continue restore, m_completedSegmentSize:%llu", m_completedSegmentSize);
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_LOAD_DIRTYRANGES);
        return SUCCESS;
    }
    ReportJobSpeed(m_completedDataSize);
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_POSTHOOK);
    return SUCCESS;
}
void RestoreJob::ArchiveTaskInitial(DirtyRanges::iterator& it, std::shared_ptr<RestoreIOTask>& task)
{
    m_getFileReq.fileOffset = it->Offset();
    /** sdk requst param, 0:4M; 1:8M... **/
    m_getFileReq.readSize = 0;
    task = std::make_shared<RestoreIOTask>(it->Offset(),
        it->size, m_targetVolHandler, m_clientHandler, m_getFileReq);
    task->SetIgnoreBadBlockFlag(m_forceRecoverIgnoreBadBlock);
}
/**
 *  @brief 执行子任务_清理资源
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::SubTaskClean()
{
    int result = SUCCESS;
    if (m_dataRepoHandler.get() != nullptr) {
        if (m_dataRepoHandler->Close() != SUCCESS) {
            ERRLOG("Close data repo failed, %s", m_taskInfo.c_str());
        }
    }
    if (m_targetVolHandler.get() != nullptr) {
        if (m_targetVolHandler->Close() != SUCCESS) {
            ERRLOG("Close vol failed, %s", m_taskInfo.c_str());
            result = FAILED;
        }

        if (m_targetVolHandler->CleanLeftovers() != SUCCESS) {
            WARNLOG("Clean remain mount point failed.");
        }
    }
    return result;
}

/**
 *  @brief 初始化参数
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::InitExecJobParams()
{
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_subJobExtendInfo)) {
        ERRLOG("Get restore subjob info failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobExtendInfo.m_targetVolumeInfo, m_targetVolInfo)) {
        ERRLOG("Failed to get vol info, %s", m_taskInfo.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobExtendInfo.m_originVolumeInfo, m_originVolInfo)) {
        ERRLOG("Failed to get vol info, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Begin to exec origin vol(%s) restore to target vol(%s), %s", m_originVolInfo.m_uuid.c_str(),
        m_targetVolInfo.m_uuid.c_str(), m_taskInfo.c_str());
    // 从配置文件中获取参数
    m_maxIOThreadNumConf = Module::ConfigReader::getUint(VOLUME_DATA_PROCESS, MAX_BACKUP_THREADS);
    if (m_maxIOThreadNumConf <= 0) {
        m_maxIOThreadNumConf = DEFAULT_BACKUP_THREADS;
    }
    m_segmentSizeConf = GetSegementSizeFromConf();
    std::string forceRecoverFlag = Module::ConfigReader::getString("General", "RecoverIgnoreBadBlock");
    m_forceRecoverIgnoreBadBlock = true ? forceRecoverFlag == "yes" : false;
    return SUCCESS;
}

/**
 *  @brief 创建Handler
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::CreateIOHandler()
{
    if (CreateReadHandler() != SUCCESS || CreateWriteHandler() != SUCCESS) {
        ERRLOG("CreateIOHandler failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Success to create IOHandler, %s", m_taskInfo.c_str());
    return SUCCESS;
}
/**
 *  @brief 创建ReadHandler
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::CreateReadHandler()
{
    if (m_dataRepoHandler.get() == nullptr) {
        ERRLOG("The data repo handler is null, %s", m_taskInfo.c_str());
        return FAILED;
    }
    std::string readFileName = m_dataRepoPath + Module::PATH_SEPARATOR + m_originVolInfo.m_uuid + ".raw";
    if (m_dataRepoHandler->Open(readFileName, "r") != SUCCESS) {
        ERRLOG("Open data repo failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    INFOLOG("Success to create IOHandler, %s", m_taskInfo.c_str());
    return SUCCESS;
}
/**
 *  @brief 创建生产端Handler
 *
 *  @return 0 成功，非0 失败
 */
int RestoreJob::CreateWriteHandler()
{
    if (m_protectEngine->GetVolumeHandler(m_targetVolInfo, m_targetVolHandler) != SUCCESS
        || m_targetVolHandler.get() == nullptr) {
        ERRLOG("Failed to get target volHandler handler for IOWrite, %s", m_taskInfo.c_str());
        return FAILED;
    }
    auto handler = std::bind(&RestoreJob::ReportApplicationLabels, this, std::placeholders::_1);
    m_targetVolHandler->SetReportJobDetailHandler(handler);
    // 打开handler
    if (m_targetVolHandler->Open(VolOpenMode::READ_WRITE) != SUCCESS) {
        ERRLOG("Open vol failed, %s", m_taskInfo.c_str());
        VolHandlerReportTaskLabel(m_targetVolHandler);
        return FAILED;
    }
    INFOLOG("Success to create WriteHandler, %s", m_taskInfo.c_str());
    return SUCCESS;
}
/**
 *  @brief 查询io任务执行结果
 *
 *  @return true 成功，false 失败
 */
bool RestoreJob::IOTaskExecResult(int &nTaskExecuting)
{
    bool result = true;
    std::shared_ptr<BlockTask> execTask;
    while (m_taskScheduler->Get(execTask, IO_TIME_OUT)) {
        --nTaskExecuting;
        if (execTask->Result() != SUCCESS) {
            ERRLOG("Task failed, %s", m_taskInfo.c_str());
            result = false;
        }
        ++m_completedBlocks;  // 已处理完的Block数量加一
        m_completedDataSize += DIRTY_RANGE_BLOCK_SIZE;
    }
    return result;
}

/**
 *  @brief 处理尚未结束的任务的执行结果
 *
 *  @return 无
 */
void RestoreJob::ExecIOTaskEnd(int &nTaskExecuting, int &taskExecRet)
{
    std::shared_ptr<BlockTask> execTask;
    while (nTaskExecuting > 0) {
        if (!m_taskScheduler->Get(execTask)) {
            taskExecRet = FAILED;
            break;
        }
        if (execTask->Result() != SUCCESS) {
            taskExecRet = FAILED;
        }
        --nTaskExecuting;
        ++m_completedBlocks;  // 已处理完的Block数量加一
        m_completedDataSize += DIRTY_RANGE_BLOCK_SIZE;
    }
}

bool RestoreJob::GetSnapshotInfo(VolSnapInfo &snapInfo)
{
    SnapshotInfo snapshotInfo;
    if (m_isArchiveRestore) {
        DBGLOG("Load copy snapshot by cacheRepoPath%s. %s", m_cacheRepoPath.c_str(), m_taskInfo.c_str());
        std::string snapshotFile = m_cacheRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
        if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, snapshotFile, snapshotInfo) != SUCCESS) {
            ERRLOG("Load copy snapshot info failed, %s", m_taskInfo.c_str());
            return false;
        }
    } else {
        DBGLOG("Load copy snapshot by metaRepoPath.%s. %s", m_metaRepoPath.c_str(), m_taskInfo.c_str());
        std::string snapshotFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
        if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, snapshotFile, snapshotInfo) != SUCCESS) {
            ERRLOG("Load copy snapshot info failed, %s", m_taskInfo.c_str());
            return false;
        }
    }
    bool result = false;
    for (int i = 0; i < snapshotInfo.m_volSnapList.size(); i++) {
        VolSnapInfo curVolSnapshot = snapshotInfo.m_volSnapList[i];
        if (curVolSnapshot.m_volUuid == m_originVolInfo.m_uuid) {
            snapInfo = curVolSnapshot;
            result = true;
            break;
        }
    }
    return result;
}

// --------------------------------------- Generate sub task-----------------------------------------------------------
void RestoreJob::SetGenerateJobStateMachine()
{
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT)] =
        std::bind(&RestoreJob::GenerateJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PREHOOK)] =
        std::bind(&RestoreJob::GenerateSubJobPreHook, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_GET_VOL_PAIR_LIST)] =
        std::bind(&RestoreJob::GetVolMatchPairInfo, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_EXEC_JOB)] =
        std::bind(&RestoreJob::CreateSubTasksByVolPairInfo, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_POST_SUB_JOB)] =
        std::bind(&RestoreJob::CreatePostSubJob, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_PUT_JOB_TO_FRAME)] =
        std::bind(&RestoreJob::PutSubTasksToFrame, this);
    m_stateHandles[static_cast<int>(VirtPlugin::State::STATE_GEN_POSTHOOK)] =
        std::bind(&RestoreJob::GenerateSubJobPostHook, this);
}

int RestoreJob::GenerateJobInit()
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

int RestoreJob::GetVolMatchPairInfo()
{
    std::string infoPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_MATCH_INFO;
    if (Utils::LoadFileToStructWithRetry(m_cacheRepoHandler, infoPath, m_finalRestoreVolPair) != SUCCESS) {
        ERRLOG("Load vm info file to struct failed, %s", m_taskInfo.c_str());
        return FAILED;
    }

    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_EXEC_JOB);
    return SUCCESS;
}

int RestoreJob::CreateSubTasksByVolPairInfo()
{
    int index = 0;
    for (VolPair& volPair : m_finalRestoreVolPair.m_volPairList) {
        VolInfo& srcVolInfo = volPair.m_originVol;
        srcVolInfo.m_metadata = ""; // 防止字符串过长导致后面上报子任务失败
        VolInfo& dstVolInfo = volPair.m_targetVol;
        SubJob subJob {};
        subJob.__set_jobId(m_jobId);
        subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
        subJob.__set_jobName(BUSINESS_SUB_JOB_NAME_PREFIX + std::to_string(index));
        subJob.__set_policy(ExecutePolicy::ANY_NODE);
        int priority = 1;
        subJob.__set_jobPriority(priority);
        SubJobExtendInfo eInfo;
        if (!Module::JsonHelper::StructToJsonString(srcVolInfo, eInfo.m_originVolumeInfo)) {
            ERRLOG("Src vol info fomat error, %s", m_taskInfo.c_str());
            return FAILED;
        }
        if (!Module::JsonHelper::StructToJsonString(dstVolInfo, eInfo.m_targetVolumeInfo)) {
            ERRLOG("Dst vol info fomat error, %s", m_taskInfo.c_str());
            return FAILED;
        }
        std::string infoStr = "";
        if (!Module::JsonHelper::StructToJsonString(eInfo, infoStr)) {
            ERRLOG("SubJob extend info fomat error, %s", m_taskInfo.c_str());
            return FAILED;
        }
        subJob.__set_jobInfo(infoStr);
        m_execSubs.push_back(subJob);
        ++index;
    }

    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_CREATE_POST_SUB_JOB);
    return SUCCESS;
}


int RestoreJob::CreatePostSubJob()
{
    SubJob subJob {};
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("PostSubJob");
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    int priority = 2;
    subJob.__set_jobPriority(priority);
    subJob.__set_jobInfo("{}");
    m_execSubs.push_back(subJob);

    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_PUT_JOB_TO_FRAME);
    return SUCCESS;
}

int RestoreJob::PutSubTasksToFrame()
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

int RestoreJob::GenMainTaskStatusToFile()
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

bool RestoreJob::CheckMainTaskStatusFileExist()
{
    std::string mainTaskStatusInfoFile = m_cacheRepoPath + VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO;
    return m_cacheRepoHandler->Exists(mainTaskStatusInfoFile);
}

int RestoreJob::PrerequisitePreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.nextState = static_cast<int>(State::STATE_PRE_CHECK_BEFORE_RECOVER);
    para.postHookState = static_cast<int>(State::STATE_PRE_POSTHOOK);
    return ExecHook(para);
}

int RestoreJob::PrerequisitePostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::PRE_PREREQUISITE;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int RestoreJob::GenerateSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.nextState = static_cast<int>(State::STATE_GEN_GET_VOL_PAIR_LIST);
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.postHookState = static_cast<int>(State::STATE_GEN_POSTHOOK);
    return ExecHook(para);
}

int RestoreJob::GenerateSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::GENERATE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int RestoreJob::ExecuteSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_EXECJOB_INITIALIZE);
    para.postHookState = static_cast<int>(State::STATE_EXECJOB_POSTHOOK);
    return ExecHook(para);
}

int RestoreJob::ExecuteSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int RestoreJob::PostSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_POST_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_EXEC_POST_CREATE_MACHINE);
    para.postHookState = static_cast<int>(State::STATE_EXEC_POST_SUBJOB_POSTHOOK);
    return ExecHook(para);
}

int RestoreJob::PostSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_POST_SUB_JOB;
    para.nextState = static_cast<int>(State::STATE_NONE);
    return ExecHook(para);
}

int RestoreJob::PostJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::POST_JOB;
    if (m_jobResult == AppProtect::JobResult::type::SUCCESS) {
        para.jobExecRet = SUCCESS;
        para.nextState = static_cast<int>(State::STATE_POST_CLEAN_RESOURCE);
    } else {
        para.jobExecRet = FAILED;
        if (m_RestoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
            para.nextState = static_cast<int>(State::STATE_POST_DELETE_MACHINE);
        } else {
            para.nextState = static_cast<int>(State::STATE_POST_DELETE_VOLUMES);
        }
    }
    para.postHookState = static_cast<int>(State::STATE_POST_POSTHOOK);
    int ret = ExecHook(para);
    ReportTaskLabel();
    return ret;
}

int RestoreJob::PostJobPostHook()
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
    return ExecHook(para);
}
}
