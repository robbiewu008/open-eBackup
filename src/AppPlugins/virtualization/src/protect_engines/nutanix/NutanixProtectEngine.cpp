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
#include <set>
#include <map>
#include <unordered_map>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <common/JsonHelper.h>
#include "common/Utils.h"
#include "volume_handlers/cloud_volume/nutanix_volume/NutanixVolumeHandler.h"
#include "NutanixProtectEngine.h"

using namespace VirtPlugin;
using namespace NutanixPlugin::NutanixErrorCode;

namespace {
    const int32_t COMMON_WAIT_TIME_3S = 3;
    const int32_t NUTANIX_MAX_TIME_OUT = 720000; // 防呆设置最大200小时超时时间
    const std::string NUTANIX_CONF = "NutanixConfig";
    const int32_t NUTANIX_CERT_NOT_EXIST = 1577090049;
    const uint64_t NUTANIX_MB_TO_BYTE = 1048576;
    std::map<int64_t, std::string> NUTANIX_LABEL_DICT = {
        { 14, "virtual_plugin_cnware_memory_limit_label" }
    };
    const int32_t HOST_DISCONNECT = 1;
    const int32_t HOST_MAINTAIN = 2;
    const int32_t HOST_ERR = 3;
    std::map<int32_t, std::string> HOST_STATUS_MAP = {
        { 0, "Normal" },
        { 1, "Disconnected" },
        { 2, "Maintaining" },
        { 3, "Error" }
    };
    const int VM_NAME_LIMIT = 80;
    const std::string NEW_VM_INFO_FILE = VirtPlugin::VIRT_PLUGIN_CACHE_ROOT + "new_vm.info";
    const std::string NUTANIX_POWERSTATE_MIGRATING = "MIGRATING";
    const std::string RESTRICTED_CHARACTERS =  "^(?!.*[*,'\"\\[\\]?]).*$";
}

namespace NutanixPlugin {
void NutanixProtectEngine::SetAppEnv(const ApplicationEnvironment &appEnv)
{
    m_appEnv = appEnv;
    return;
}

bool NutanixProtectEngine::Init()
{
    if (m_nutanixClient == nullptr) {
        m_nutanixClient = std::make_shared<NutanixClient>(m_appEnv.auth);
    }
    m_certMgr = std::make_shared<CertManger>();
    if (m_certMgr == nullptr) {
        ERRLOG("CertMgr is nullptr, %s", m_jobId.c_str());
        return false;
    }
    CertInfo cert;
    if (!m_certMgr->ParseCert(m_appEnv.endpoint, m_appEnv.auth.extendInfo, cert)) {
        ERRLOG("Parse Cert failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    m_domainId = Utils::GetProxyHostId(true);
    return (m_nutanixClient != nullptr);
}

bool NutanixProtectEngine::InitClient(int64_t& errorCode, std::string &errorDes)
{
    if (m_nutanixClient == nullptr) {
        ERRLOG("CNware Client nullptr! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    if (m_nutanixClient->Init(m_appEnv) != SUCCESS) {
        ERRLOG("Build login body failed! Taskid: %s", m_appEnv.id.c_str());
        errorCode = INIT_CLIENT_FAILED;
        return false;
    }
    return true;
}

void NutanixProtectEngine::SetCommonInfo(NutanixRequest& req)
{
    if (m_certMgr == nullptr) {
        ERRLOG("SetCommonInfo m_certMgr nullptr! Taskid: %s", m_jobId.c_str());
        return;
    }
    AuthObj authObj;
    authObj.name = m_appEnv.auth.authkey;
    authObj.passwd = m_appEnv.auth.authPwd;
    authObj.certVerifyEnable = m_certMgr->IsVerifyCert();
    authObj.cert = m_certMgr->GetCertPath();
    authObj.revocationList = m_certMgr->GetRevocationListPath();
    req.SetEndpoint(m_appEnv.endpoint);
    req.SetEnvAddress(m_appEnv.endpoint);
    req.SetIpPort(std::to_string(m_appEnv.port));
    req.SetUserInfo(authObj);
    return;
}

void NutanixProtectEngine::ListApplicationResourceV2(ResourceResultByPage& page, const ListResourceRequest& request)
{
    SetAppEnv(request.appEnv);
    int64_t errorCode(0);
    if (!Init()) {
        ERRLOG("Init Nutanix engine failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    std::string errorDes;
    if (!InitClient(errorCode, errorDes)) {
        ERRLOG("InitClient failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    std::shared_ptr<NutanixResourceManager> ResourceMgrPtr = std::make_shared<NutanixResourceManager>(
        request.appEnv, request.condition, m_nutanixClient, m_certMgr);
    if (ResourceMgrPtr == nullptr) {
        ERRLOG("Init ResourceMgrPtr failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    if (ResourceMgrPtr->GetTargetResource(page) != SUCCESS) {
        ERRLOG("Get target resource failed! Subtype: %s", request.appEnv.subType.c_str());
        return;
    }
    return;
}

bool NutanixProtectEngine::InitRestoreJobPara(std::shared_ptr<ThriftDataBase> &jobInfo)
{
    m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo);
    if (m_restorePara == nullptr) {
        ERRLOG("Get restore job parameter failed, %s", m_taskId.c_str());
        return false;
    }
    m_requestId = m_restorePara->requestId;
    m_appEnv = m_restorePara->targetEnv;
    m_application = m_restorePara->targetObject;
    m_subObjects = m_restorePara->restoreSubObjects;
    if (!Module::JsonHelper::JsonStringToStruct(m_restorePara->extendInfo, m_restoreExtendInfo)) {
        ERRLOG("Convert %s extendInfo failed, %s", WIPE_SENSITIVE(m_restorePara->extendInfo).c_str(), m_jobId.c_str());
        return false;
    }
    if (!m_restoreExtendInfo.restoreLevel.empty()) {
        m_restoreLevel = RestoreLevel(std::stoi(m_restoreExtendInfo.restoreLevel));
    }
    return true;
}

std::string NutanixProtectEngine::GetTaskId() const
{
    return "TaskId=" + m_requestId + ".";
}

bool NutanixProtectEngine::InitJobPara()
{
    INFOLOG("Enter Nutanix InitjobPara");
    if (m_initialized && m_backupPara != nullptr && m_restorePara != nullptr) {
        DBGLOG("Initialized, return directly, %s", m_taskId.c_str());
        return true;
    }

    if (m_jobHandle == nullptr || m_jobHandle->GetJobCommonInfo() == nullptr) {
        ERRLOG("Job handler or job common info is null, %s", m_taskId.c_str());
        return false;
    }

    std::shared_ptr<ThriftDataBase> jobInfo = m_jobHandle->GetJobCommonInfo()->GetJobInfo();
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        m_backupPara = std::dynamic_pointer_cast<AppProtect::BackupJob>(jobInfo);
        if (m_backupPara == nullptr) {
            ERRLOG("Get backup job parameter failed, %s", m_taskId.c_str());
            return false;
        }
        m_requestId = m_backupPara->requestId;
        m_appEnv = m_backupPara->protectEnv;
        m_application = m_backupPara->protectObject;
        m_subObjects = m_backupPara->protectSubObject;
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE) {
        if (!InitRestoreJobPara(jobInfo)) {
            ERRLOG("Init restore job parameter failed. %s", m_taskId.c_str());
            return false;
        }
    } else if (m_jobHandle->GetJobType() == JobType::CANCELLIVEMOUNT) {
            return false;
    } else if (m_jobHandle->GetJobType() != JobType::LIVEMOUNT) {
        return false;
    }
    m_taskId = GetTaskId();
    m_initialized = true;

    if (m_jobHandle->GetJobType() != JobType::LIVEMOUNT) {
        INFOLOG("Job is not livemount.");
        return ConnectToEnv(m_appEnv) == SUCCESS;
    }

    INFOLOG("Initialize Nutanix protect engine job parameter handler success, %s", m_taskId.c_str());
    return true;
}

int32_t NutanixProtectEngine::PreHook(const ExecHookParam &para)
{
    INFOLOG("Enter nutanix prehook");
    if (!InitJobPara()) {
        ERRLOG("InitJobPara, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool NutanixProtectEngine::DetachVolsOnVM(const std::vector<std::string> &vols, const std::string &vmId)
{
    DetachDiskRequest req(vmId, vols);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, DetachDiskRequest>(req);
    if (response == nullptr || !CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Detach disk on vm %s failed, %s", vmId.c_str(), m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::CleanTmpDiskOnAgent()
{
    INFOLOG("Enter nutanix CleanTmpDiskOnAgent, %s", m_taskId.c_str());
    InitRepoHandler();
    int32_t iRet = SUCCESS;
    std::vector<std::string> tmpDiskInfoFiles;
    std::string tmpVolInfoPath = m_cacheRepoPath + VIRT_PLUGIN_NEW_VOLUME_INFO;
    m_cacheRepoHandler->GetFiles(tmpVolInfoPath, tmpDiskInfoFiles);
    INFOLOG("TmpDiskInfoFiles size: %d", tmpDiskInfoFiles.size());
    std::unordered_map<std::string, std::vector<std::string>> agentTmpDiskMap;
    for (const auto &tmpDiskInfoFile: tmpDiskInfoFiles) {
        NewVolumeInfo tmpVolInfo;
        if (Utils::LoadFileToStruct(m_cacheRepoHandler, tmpDiskInfoFile, tmpVolInfo) != SUCCESS) {
            ERRLOG("Failed to read meta file %s", tmpDiskInfoFile.c_str());
            iRet = FAILED;
        }
        agentTmpDiskMap[tmpVolInfo.agentVmUuid].push_back(tmpVolInfo.tmpVolUuid);
    }
    for (auto &agentTmpDisk : agentTmpDiskMap) {
        if (!DetachVolsOnVM(agentTmpDisk.second, agentTmpDisk.first)) {
            ERRLOG("Detach tmp disk on agent failed.");
            iRet = FAILED;
        }
    }
    INFOLOG("Exit nutanix CleanTmpDiskOnAgent, ret %d, %s", iRet, m_taskId.c_str());
    return iRet;
}

int32_t NutanixProtectEngine::PostHook(const ExecHookParam &para)
{
    if (m_jobHandle->GetJobType() == JobType::RESTORE && para.hookType == HookType::POST_HOOK
        && para.stage == JobStage::POST_JOB) {
        return CleanTmpDiskOnAgent();
    }
    return SUCCESS;
}

// snapshot
int32_t NutanixProtectEngine::CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode)
{
    INFOLOG("Enter nutanix createsnapshot");
    if (!DoGetMachineMetadata(m_backupPara->protectObject.id)) {
        ERRLOG("Set VM metadata info failed, %s", m_taskId.c_str());
        return FAILED;
    }

    if (m_vmInfo.m_volList.empty()) {
        ERRLOG("Failed, volume list is empty.task id: %s", m_taskId.c_str());
        return FAILED;
    }
    snapshot.m_moRef = Uuid::GenerateUuid();
    snapshot.m_vmMoRef = m_vmInfo.m_moRef;
    snapshot.m_vmName = m_vmInfo.m_name;

    DBGLOG("snapshot.m_moRef: %s, snapshot.m_vmMoRef: %s, snapshot.m_vmName: %s. %s", snapshot.m_moRef.c_str(),
        snapshot.m_vmMoRef.c_str(), snapshot.m_vmName.c_str(), m_taskId.c_str());

    NutanixCreateSnapshotRequest req(snapshot.m_moRef, m_vmInfo.m_uuid);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, NutanixCreateSnapshotRequest>(req);
    if (response == nullptr) {
        ERRLOG("Create Vm snapshot failed, %s", m_taskId.c_str());
        return FAILED;
    } else {
        if (!GetSnapshotResult(response->GetResult().taskId, snapshot.m_moRef, snapshot, errCode)) {
            ERRLOG("Get snapshot result failed, %s", m_taskId.c_str());
            return FAILED;
        }
    }
    snapshot.m_snapFlag = response->GetResult().taskId;
    return SUCCESS;
}

bool NutanixProtectEngine::DoDeleteSnapshot(const std::string &snapshotId)
{
    INFOLOG("Enter DoDeleteSnapshot");
    DelSnapshotRequest req(snapshotId);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, DelSnapshotRequest>(req);
    if (response == nullptr) {
        ERRLOG("send del snapshot request failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Delete snapshot on Vm failed. id: %s, %s", snapshotId.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Delete snapshot %s success, %s", snapshotId.c_str(), m_taskId.c_str());
    return true;
}

int32_t NutanixProtectEngine::DeleteSnapshot(const SnapshotInfo &snapshot)
{
    if (snapshot.m_moRef.empty()) {
        ERRLOG("snapshot id is empty. %s", m_taskId.c_str());
        return FAILED;
    }
    if (!DoDeleteSnapshot(snapshot.m_moRef)) {
        ERRLOG("Delete snapshot(%s) failed, %s", snapshot.m_moRef.c_str(), m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::QuerySnapshotExists(SnapshotInfo &snapshot)
{
    snapshot.m_deleted = true;
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetMachineMetadata(VMInfo &vmInfo)
{
    std::string vmId;
    if (m_jobHandle->GetJobType() == JobType::BACKUP) {
        vmId = m_backupPara->protectObject.id;
    } else {
        vmId = m_restorePara->targetObject.id;
    }
    if (!DoGetMachineMetadata(vmId)) {
        ERRLOG("Get machine metadata failed, %s", m_taskId.c_str());
        return FAILED;
    }
    vmInfo = m_vmInfo;
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetVolumesMetadata(const VMInfo &vmInfo,
    std::unordered_map<std::string, std::string> &volsMetadata)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler)
{
    INFOLOG("Enter Nutanix GetVolumeHandler");
    std::shared_ptr<NutanixVolumeHandler> NutanixVolHandler =
    std::make_shared<NutanixVolumeHandler>(GetJobHandle(), volInfo, m_jobId, m_subJobId);
    if (NutanixVolHandler->InitializeVolumeInfo(NUTANIX_CONF) != SUCCESS ||
        !NutanixVolHandler->InitClient()) {
        ERRLOG("Initialize volume info failed.");
        return FAILED;
    }
    volHandler = NutanixVolHandler;

    return SUCCESS;
}

int32_t NutanixProtectEngine::DetachVolume(const VolInfo &volObj)
{
    WARNLOG("New created vol: %s (id: %s), will be detached in post subJob.%s",
        volObj.m_name.c_str(), volObj.m_uuid.c_str(), m_jobId.c_str());
    return SUCCESS;
}

bool NutanixProtectEngine::LoadNewVolumeFromFile(const VolInfo &volObj, NewVolumeInfo &newVolInfo)
{
    std::string newVolumeInfoFile = m_cacheRepoPath + VIRT_PLUGIN_NEW_VOLUME_INFO + "/" + volObj.m_name + ".info";
    if (Utils::LoadFileToStruct(m_cacheRepoHandler, newVolumeInfoFile, newVolInfo) != SUCCESS) {
        ERRLOG("Failed to read meta file %s", newVolumeInfoFile.c_str());
        return false;
    }
    return true;
}

void NutanixProtectEngine::SetCloneByIdDiskInfo(AttachCloneByIdVmDisk &disk, const NewVolumeInfo &newVolInfo,
    const VolInfo &volObj)
{
    disk.diskCloneById.diskAddressCloneById.vmDiskUuid = newVolInfo.tmpVolUuid;
    disk.diskAddress.deviceBus = volObj.m_type;
    disk.diskAddress.deviceIndex = std::stoi(volObj.m_slotId);
    disk.storageContainerUuid = volObj.m_datastore.m_moRef;
}

bool NutanixProtectEngine::GetTargetVmId(std::string &vmId)
{
    std::string newVMMetaDataPath = m_cacheRepoPath + NEW_VM_INFO_FILE;
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_DISK) {
        INFOLOG("Disk restore no need attach volume.");
        return true;
    } else {
        VMInfo targetVm;
        if (VirtPlugin::Utils::LoadFileToStruct(m_cacheRepoHandler, newVMMetaDataPath, targetVm) != SUCCESS) {
            ERRLOG("Load file %s to struct failed,", newVMMetaDataPath.c_str());
            return false;
        }
        vmId = targetVm.m_uuid;
        return true;
    }
}

bool NutanixProtectEngine::AttachVol2TargetVmWithCloneMode(const NewVolumeInfo &tmpVolInfo, const VolInfo &volObj)
{
    std::string targetVmId;
    if (!GetTargetVmId(targetVmId)) {
        ERRLOG("Get target Vm Id failed");
        return false;
    }
    AttachDiskRequest req(targetVmId);
    SetCommonInfo(req);
    AttachCloneByIdVmDisk disk;
    SetCloneByIdDiskInfo(disk, tmpVolInfo, volObj);
    req.SetCloneByIdVmDisk(disk, tmpVolInfo.agentVmUuid);

    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI< NutanixResponse<struct TaskMsg>, AttachDiskRequest>(req);
    if (response == nullptr || !CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Attach disk to target vm failed, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::AttachVolume(const VolInfo &volObj)
{
    INFOLOG("Enter");
    InitRepoHandler();
    NewVolumeInfo tmpVolInfo;
    if (!LoadNewVolumeFromFile(volObj, tmpVolInfo)) {
        ERRLOG("Load new volume from file failed, %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Attach volume bus : %d ", tmpVolInfo.busIndex);
    if (!AttachVol2TargetVmWithCloneMode(tmpVolInfo, volObj)) {
        ERRLOG("Failed to attach volume to target vm with clone mode, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::DeleteVolume(const VolInfo &volObj)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::ReplaceVolume(const VolInfo &volObj)
{
    return SUCCESS;
}


bool NutanixProtectEngine::GetTargetNic(const std::string &originNicId, const std::vector<VmNic> &vmNics,
    NewVMNicStruct &newNic)
{
    for (const struct VmNic &srcIt : vmNics) {
        if (srcIt.nicUuid == originNicId) {
            DBGLOG("Find nic id: %s", originNicId.c_str());
            newNic.model = srcIt.model;
            newNic.vlanMode = srcIt.vlanMode;
            return true;
        }
    }
    ERRLOG("Not find target nic, origin id: %s", originNicId.c_str());
    return false;
}

int32_t NutanixProtectEngine::FormatDomainInterface(struct NutanixVMInfo &domainInfo,
    std::vector< struct NewVMNicStruct > &newVMNics)
{
    INFOLOG("Enter");
    if (m_restorePara == nullptr) {
        ERRLOG("Get m_restorePara pointer null. %s", m_jobId.c_str());
        return FAILED;
    }
    if (domainInfo.vmNics.empty()) {
        INFOLOG("Empty interface info from copy");
        return SUCCESS;
    }
    RestoreNICList TargetNicList;
    if (!Module::JsonHelper::JsonStringToStruct(m_restoreExtendInfo.bridgeInterface, TargetNicList)) {
        ERRLOG("Get TargetNicList info failed. %s", m_jobId.c_str());
        return FAILED;
    }
    bool needOpenInterface = m_restoreExtendInfo.openInterface == "true";
    for (const auto &it : TargetNicList.m_detail) {
        DBGLOG("Target network id: %s", it.targetNetworkId.c_str());
        NewVMNicStruct tmpVmNic;
        if (!GetTargetNic(it.originNicId, domainInfo.vmNics, tmpVmNic)) {
            ERRLOG("Match origin nic failed, id: %s", it.originNicId.c_str());
            return FAILED;
        }
        tmpVmNic.requestedIpAddress = it.targetIp;
        tmpVmNic.networkUuid = it.targetNetworkId;
        tmpVmNic.isConnected = needOpenInterface;
        newVMNics.push_back(tmpVmNic);
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::FormatCreateMachineParam(VMInfo &vmInfo, struct NutanixVMInfo &domainInfo,
    std::vector<struct NewVMNicStruct> &newVMNics)
{
    // 获取原虚拟机信息;
    if (!Module::JsonHelper::JsonStringToStruct(vmInfo.m_metadata, domainInfo)) {
        ERRLOG("Get new domain info failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (GetNewVMMachineName(domainInfo.name) != SUCCESS) {
        ERRLOG("Get new vm machine name failed. %s", m_jobId.c_str());
        return FAILED;
    }
    vmInfo.m_name = domainInfo.name;
    PMInterfaceHostUuidList hostUuidList;
    if (!Module::JsonHelper::JsonStringToStruct(m_restoreExtendInfo.hostUuids,
        hostUuidList)) {
        ERRLOG("Convert %s failed, %s", WIPE_SENSITIVE(
            m_restoreExtendInfo.hostUuids.c_str()), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("vm info metadata: %s", vmInfo.m_metadata.c_str());
    if (!hostUuidList.m_detail.empty()) {
        domainInfo.affinity.hostUuids.clear();
        for (const auto &it : hostUuidList.m_detail) {
            domainInfo.affinity.hostUuids.emplace_back(it);
            INFOLOG("domain host id : %s", it.c_str());
        }
    }
    if (FormatDomainInterface(domainInfo, newVMNics) != SUCCESS) {
        ERRLOG("Format domain disk device info failed. %s", m_jobId.c_str());
        return FAILED;
    }

    return SUCCESS;
}

bool NutanixProtectEngine::GetNewCreatedVmId(const std::string &taskId, std::string &newVmId)
{
    QueryTaskRequest req(taskId);
    SetCommonInfo(req);
    std::string taskStatus = NUTANIX_TASK_STATUS_NONE;
    std::shared_ptr<NutanixResponse<struct NutanixTaskResponse>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixTaskResponse>, QueryTaskRequest>(req);
    std::vector<Entity> taskEntites = response->GetResult().entites;
    if (taskEntites.empty() || taskEntites[0].entityId.empty()) {
        ERRLOG("can not get new creaetd vm id from response. %s", taskId.c_str());
        return false;
    }
    newVmId = taskEntites[0].entityId;
    return true;
}

int32_t NutanixProtectEngine::DoCreateMachine(VMInfo &vmInfo, CreateVMRequest &createVmReq)
{
    INFOLOG("Enter");
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, CreateVMRequest>(createVmReq);
    if (response == nullptr || !CheckTaskStatus(response->GetResult().taskId)) {
        if (NUTANIX_LABEL_DICT.count(m_nutanixClient->GetErrorCode().errorCode.code) != 0) {
            m_reportArgs = { createVmReq.Get().name };
            m_reportParam = {
                NUTANIX_LABEL_DICT[m_nutanixClient->GetErrorCode().errorCode.code],
                JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, 0, 0 };
        }
        ERRLOG("Create vm failed. Vm name : %s", createVmReq.Get().name.c_str());
        return FAILED;
    }

    // 通过任务id查询新虚拟机id;
    std::string newVmId;
    if (!GetNewCreatedVmId(response->GetResult().taskId, newVmId)) {
        ERRLOG("Get new created vm id failed, %s", m_jobId.c_str());
        return FAILED;
    }
    vmInfo.m_uuid = newVmId;
    // 清除卷信息，查询新创建的卷重新赋值，因为Nutanix的特殊性，暂时不会创建卷;
    vmInfo.m_volList.clear();
    INFOLOG("Create vm %s id: %s success. %s", vmInfo.m_name.c_str(), vmInfo.m_uuid.c_str(), m_jobId.c_str());
    return SUCCESS;
}

bool NutanixProtectEngine::DeleteResidueVm()
{
    INFOLOG("Enter");
    InitRepoHandler();
    std::string newVMMetaDataPath = m_cacheRepoPath + NEW_VM_INFO_FILE;
    if (!m_cacheRepoHandler->Exists(newVMMetaDataPath)) {
        INFOLOG("Not exist residue virtual machine, path: %s, %s", newVMMetaDataPath.c_str(), m_jobId.c_str());
        return true;
    }
    VMInfo residueVm;
    if (VirtPlugin::Utils::LoadFileToStruct(m_cacheRepoHandler, newVMMetaDataPath, residueVm) != SUCCESS) {
        ERRLOG("Load file %s to struct failed, %s", newVMMetaDataPath.c_str(), m_jobId.c_str());
        return false;
    }
    if (DeleteMachine(residueVm) != SUCCESS) {
        WARNLOG("Delete residue failed, %s", newVMMetaDataPath.c_str(), m_jobId.c_str());
        return false;
    }
    if (!m_cacheRepoHandler->Remove(newVMMetaDataPath)) {
        ERRLOG("Remove residue Vm file failed, file:%s, %s", newVMMetaDataPath.c_str(), m_jobId.c_str());
        return false;
    }
    return false;
}

// machine
int32_t NutanixProtectEngine::CreateMachine(VMInfo &vmInfo)
{
    INFOLOG("Enter");
    // 入参为副本虚拟机信息，将id置零，防止创建失败后，误删原虚拟机;
    vmInfo.m_uuid = "";
    struct NutanixVMInfo addVmInfo;
    std::vector<struct NewVMNicStruct> newVMNics;
    if (FormatCreateMachineParam(vmInfo, addVmInfo, newVMNics) != SUCCESS) {
        ERRLOG("Format create machine param failed.");
        return FAILED;
    }
    // 多代理恢复，代理续作删除已经创建的虚拟机;true代表未创建虚拟机，打印标签;
    if (DeleteResidueVm()) {
        ApplicationLabelType labelParam(NUTANIX_CREATE_MACHINE_LABEL,
            std::vector<std::string>{addVmInfo.name});
        ReportJobDetail(labelParam);
    }
    CreateVMRequest createVmReq;
    SetCommonInfo(createVmReq);
    createVmReq.FillParameter(addVmInfo, newVMNics);
    if (DoCreateMachine(vmInfo, createVmReq) != SUCCESS) {
        ERRLOG("DoCreateMachine failed. %s", m_taskId.c_str());
        ApplicationLabelType labelParam(NUTANIX_CREATE_MACHINE_FAILED_LABEL,
            std::vector<std::string>{addVmInfo.name}, JobLogLevel::TASK_LOG_ERROR,
            CNWARE_CREATE_MACHINE_FAILED_ERROR);
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::DoDeleteMachine(const VMInfo &vmInfo)
{
    INFOLOG("Enter");
    DeleteVmRequest req(vmInfo.m_uuid);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, DeleteVmRequest>(req);
    if (response == nullptr) {
        ERRLOG("DeleteMachine failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Query DeleteMachine task status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Delete Vm %s success, %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::DeleteMachine(const VMInfo &vmInfo)
{
    if (vmInfo.m_uuid.empty()) {
        INFOLOG("No need to delete VM.");
        return SUCCESS;
    }

    if (DoDeleteMachine(vmInfo) != SUCCESS) {
        ERRLOG("delete Vm faild. id : %s, %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Delete vm %s success. %s", vmInfo.m_uuid.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::RenameMachine(const VMInfo &vmInfo, const std::string &newName)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::DoPowerOnMachine(const VMInfo &vmInfo)
{
    INFOLOG("Enter Nutanix DoPowerOnMachine");
    // 支持重复开机无需判断虚拟机状态和是否还有开机任务;
    PowerOnVmRequest req(vmInfo.m_uuid);
    SetCommonInfo(req);
    req.SetDomainId(vmInfo.m_uuid);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, PowerOnVmRequest>(req);
    if (response == nullptr) {
        ERRLOG("PowerOnMachine failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Query PowerOnMachine task status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Poweron vm %s success. %s", vmInfo.m_name.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::PowerOnMachine(const VMInfo &vmInfo)
{
    INFOLOG("Enter");
    m_reportParam.label = "";
    m_reportArgs = {};
    ApplicationLabelType labelParam;
    labelParam.label = NUTANIX_POWERON_MACHINE_LABEL;
    labelParam.params = std::vector<std::string>{ vmInfo.m_name };
    ReportJobDetail(labelParam);

    if (DoPowerOnMachine(vmInfo) != SUCCESS) {
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = NUTANIX_POWERON_MACHINE_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{ vmInfo.m_name };
        labelParam.errCode = CNWARE_POWEROFF_MACHINE_FAILED_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::DoPowerOffMachine(const VMInfo &vmInfo)
{
    // 查询虚拟机状态，如果是运行状态则关闭电源;
    NutanixVMInfo currentVMInfo;
    if (!GetVMInfoById(vmInfo.m_uuid, currentVMInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
        return FAILED;
    }
    DBGLOG("Query return vm status: %s", currentVMInfo.powerState.c_str());
    if (currentVMInfo.powerState == "OFF") {
        INFOLOG("No need to power off machine.");
        return SUCCESS;
    }
    if (currentVMInfo.powerState == "POWERING_ON" || currentVMInfo.powerState == "ON"
        || currentVMInfo.powerState == "SHUTTING_DOWN") {
        // 继续下电关机流程;
        // 考虑到虚拟机卡死无法正常关机的情况下，允许强制下电关机;
    } else if (currentVMInfo.powerState == "POWERING_OFF") {
        // 只需要去获取下电关机的任务就好;
    } else {
        // MIGRATING, UNKNOWN, PAUSED, SUSPENDING, SUSPENDED, RESUMING, RESETTING, PAUSING
        // 需要和上层DE讨论一下如何处理;
    }
    if (m_nutanixClient == nullptr) {
        ERRLOG("Query power off m_cnwareClient nullptr. %s", m_jobId.c_str());
        return FAILED;
    }
    PowerOffVmRequest poweroffReq(vmInfo.m_uuid);
    SetCommonInfo(poweroffReq);
    poweroffReq.SetDomainId(vmInfo.m_uuid);
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, PowerOffVmRequest>(poweroffReq);
    if (response == nullptr) {
        ERRLOG("Power off vm failed.");
        return FAILED;
    }
    if (!CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Query power off vm job status failed. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Power off vm success. %s", m_jobId.c_str());
    return SUCCESS;
}


int32_t NutanixProtectEngine::PowerOffMachine(const VMInfo &vmInfo)
{
    ApplicationLabelType labelParam;
    labelParam.label = NutanixErrorCode::NUTANIX_POWEROFF_MACHINE_LABEL;
    labelParam.params = std::vector<std::string>{ vmInfo.m_name };
    ReportJobDetail(labelParam);

    if (DoPowerOffMachine(vmInfo) != SUCCESS) {
        ERRLOG("DoPowerOffMachine failed. %s", m_taskId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = NutanixErrorCode::NUTANIX_POWEROFF_MACHINE_FAILED_LABEL;
        labelParam.params = std::vector<std::string>{ vmInfo.m_name };
        labelParam.errCode = NutanixErrorCode::CNWARE_POWERON_MACHINE_FAILED_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

void NutanixProtectEngine::CheckCertIsExist(int32_t &errorCode)
{
    if (m_certMgr->IsVerifyCert() && m_certMgr->GetCertPath() == "") {
        ERRLOG("The certification is not exists.");
        errorCode = NUTANIX_CERT_NOT_EXIST;
        return;
    }
    return;
}

int32_t NutanixProtectEngine::CheckProtectEnvConn(
    const AppProtect::ApplicationEnvironment &env, const std::string &vmId, int32_t &errorCode)
{
    INFOLOG("Enter Nutanix CheckProtectEnvConn");
    if (ConnectToEnv(env) != SUCCESS) {
        ERRLOG("Failed to check protect environment.");
        return FAILED;
    }
    return SUCCESS;
}

// scheduler
int32_t NutanixProtectEngine::AllowBackupInLocalNode(const AppProtect::BackupJob& job, int32_t &errorCode)
{
    INFOLOG("Enter Nutanix AllowBackupInLocalNode");
    return CheckProtectEnvConn(job.protectEnv, job.protectObject.id, errorCode);
}

int32_t NutanixProtectEngine::CheckStorageConnectionBackup(const VolInfo &volInfo, int32_t &erroCode)
{
    // 查询代理主机集群ID
    std::string agentClusterUUID;
    std::string agentHostName;
    if (QueryAgentHostClusterUUID(agentClusterUUID, agentHostName) != SUCCESS) {
        ERRLOG("Get agent cluster id failed.");
        return FAILED;
    }
    INFOLOG("agentClusterUUID : %s", agentClusterUUID.c_str());
    // 查询卷所处容器
    VirtualDiskInfo backupDiskInfo;
    if (!GetVirtualDiskInfo(volInfo.m_uuid, backupDiskInfo)) {
        ERRLOG("Get virtual disk info failed, %s", m_taskId.c_str());
        return FAILED;
    }

    NutanixStorageContainerInfo storageContainerInfo;
    if (!GetContainerInfo(backupDiskInfo.storageContainerId, storageContainerInfo)) {
        ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    ApplicationLabelType labelParam;
    labelParam.label = NUTANIX_STORAGEPOOL_CHECK_LABEL;
    labelParam.params = std::vector<std::string>{storageContainerInfo.containerId, storageContainerInfo.name};
    ReportJobDetail(labelParam);
    if (storageContainerInfo.markRemoval == true || storageContainerInfo.clusterId != agentClusterUUID) {
        INFOLOG("storageContainerInfo.clusterId : %s", storageContainerInfo.clusterId.c_str());
        // 容器被删除或者备份代理虚拟机的clusterID与容器的不一样，会导致备份代理无法访问;
        ERRLOG("Check storage pool connection failed, volume: %s, pool: %s, %s",
            volInfo.m_name.c_str(), storageContainerInfo.name.c_str(), m_jobId.c_str());
        labelParam.level = JobLogLevel::TASK_LOG_WARNING;
        labelParam.label = NUTANIX_STORAGEPOOL_NOTAVAILABLE_LABEL;
        labelParam.params = std::vector<std::string>{storageContainerInfo.containerId, storageContainerInfo.name};
        labelParam.errCode = NUTANIX_STORAGEPOOL_NOTAVAILABLE_ERROR;
        ReportJobDetail(labelParam);
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::AllowBackupSubJobInLocalNode(const AppProtect::BackupJob &job,
    const AppProtect::SubJob &subJob, int32_t &errorCode)
{
    INFOLOG("Enter Nutanix AllowBackupSubJobInLocalNode");
    if (CheckProtectEnvConn(job.protectEnv, job.protectObject.id, errorCode) != SUCCESS) {
        ERRLOG("Failed to check protect environment, taskId: %s", m_taskId.c_str());
        return FAILED;
    }
    if (subJob.jobName == REPORT_COPY_SUB_JOB) {
        return SUCCESS;
    }
    if (subJob.jobType == SubJobType::type::POST_SUB_JOB) {
        return SUCCESS;
    }
    BackupSubJobInfo backupSubJob {};
    if (!Module::JsonHelper::JsonStringToStruct(subJob.jobInfo, backupSubJob)) {
        ERRLOG("Get backup subjob info failed, %s", m_taskId.c_str());
        return FAILED;
    }
    int32_t erro;
    if (CheckStorageConnectionBackup(backupSubJob.m_volInfo, erro) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::AllowRestoreInLocalNode(const AppProtect::RestoreJob& job, int32_t &errorCode)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
    const AppProtect::SubJob& subJob, int32_t &errorCode)
{
    return SUCCESS;
}

bool NutanixProtectEngine::CheckVMStatus()
{
    INFOLOG("Enter nutanix CheckVMStatus");
    NutanixVMInfo nutanixVmInfo;
    std::string objectId;

    if (m_jobHandle->GetJobType() == JobType::BACKUP && m_backupPara != nullptr) {
        objectId = m_backupPara->protectObject.id;
        if (!GetVMInfoById(m_backupPara->protectObject.id, nutanixVmInfo)) {
            ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
            return false;
        }
        if (nutanixVmInfo.powerState == NUTANIX_POWERSTATE_MIGRATING) {
            ERRLOG("Target Vm is not in status for backup, state : %s", nutanixVmInfo.powerState);
            return false;
        }
    } else if (m_jobHandle->GetJobType() == JobType::RESTORE && m_restorePara != nullptr) {
        INFOLOG("Create new server, dont check server status.");
        return true;
    } else {
        ERRLOG("check vm status failed, jobtype: %s. %s", m_jobHandle->GetJobType(), m_taskId.c_str());
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::CheckBeforeBackup()
{
    INFOLOG("Enter nutanix checkbeforebackup");
    ApplicationLabelType hostCheckLabel;
    hostCheckLabel.label = CNWARE_BACKUP_CHECK_LABEL;
    hostCheckLabel.params = std::vector<std::string>{};
    ReportJobDetail(hostCheckLabel);
    if (!CheckVMStatus()) {
        ERRLOG("Check VM Status Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool NutanixProtectEngine::CheckHostStatus(int32_t &statusFlag, const std::string &hostId)
{
    if (m_nutanixClient == nullptr) {
        ERRLOG("CheckDiskType para nullptr! %s", m_jobId.c_str());
        statusFlag = HOST_ERR;
        return false;
    }
    GetHostInfoRequest req(hostId);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct HostInfo>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct HostInfo>, GetHostInfoRequest>(req);
    if (response == nullptr) {
        ERRLOG("GetHostInfoResponse failed! host id: %s", hostId.c_str());
        statusFlag = HOST_ERR;
        return false;
    }
    const HostInfo &hostInfo = response->GetResult();
    if (hostInfo.isMaintain) {
        ERRLOG("CheckHostStatus failed! Maintain:%d, id: %s", static_cast<int>(hostInfo.isMaintain), hostId.c_str());
        statusFlag = HOST_MAINTAIN;
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::GetCopyHostArch(const VMInfo &vmObj, std::string &copyArch)
{
    DBGLOG("Enter");
    struct NutanixVMMetaDataStruct VMMetaInfo;
    if (!Module::JsonHelper::JsonStringToStruct(vmObj.m_metadata, VMMetaInfo)) {
        ERRLOG("Get copy vm metadata info failed. %s", m_taskId.c_str());
        return FAILED;
    }
    copyArch = VMMetaInfo.clusterInfo.arch;
    INFOLOG("Copy host arch: %s", copyArch.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetTargetHostArch(std::string &targetArch)
{
    // 获取目的端的ClusterID;
    GetClusterRequest req(m_appEnv.id);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct ClusterListResponse>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct ClusterListResponse>, GetClusterRequest>(req);
    if (response == nullptr) {
        ERRLOG("Get cluster %s info failed. %s", m_appEnv.id.c_str(), m_jobId.c_str());
        return FAILED;
    }
    targetArch = response->GetResult().arch;
    INFOLOG("Target cluster %s arch: %s, %s", m_appEnv.id.c_str(), targetArch.c_str(), m_jobId.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckHostArchitectures(const VMInfo &vmObj, std::string &copyArch,
    std::string &targetArch)
{
    DBGLOG("Enter");
    if (GetCopyHostArch(vmObj, copyArch) != SUCCESS) {
        ERRLOG("Get copy host arch failed.");
        return FAILED;
    }
    uint32_t iTargetHostArch = 0;
    if (GetTargetHostArch(targetArch) != SUCCESS) {
        ERRLOG("Get target host arch failed.");
        return FAILED;
    }
    if (copyArch.empty() || targetArch.empty()) {
        ERRLOG("Arch not match, copy arch (%s), target arch (%s), taskID:%s",
            copyArch.c_str(), targetArch.c_str(), m_taskId.c_str());
        return FAILED;
    }
    if (copyArch != targetArch) {
        ERRLOG("Arch not match, copy arch (%s), target arch (%s), taskID:%s",
            copyArch.c_str(), targetArch.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("CheckHostArchitectures success, copy arch (%s), target arch (%s). %s",
        copyArch.c_str(), targetArch.c_str(), m_taskId.c_str());
    return SUCCESS;
}

bool NutanixProtectEngine::CheckHostBeforeRecover(const VMInfo &vmObj)
{
    ApplicationLabelType hostCheckLabel;
    int32_t statusFlag = 0;
    std::string hostId;

    if (m_application.subType == "NutanixHost") {
        hostId = m_application.id;
    } else if (m_application.subType == "NutanixVm") {
        hostId = m_application.parentId;
    } else {
        hostId = "";
    }

    if (hostId.empty()) {
        WARNLOG("Host id is empty, dont check host status.");
        return true;
    }
    if (!CheckHostStatus(statusFlag, hostId)) {
        ERRLOG("Check host Status Failed, %s", m_taskId.c_str());
        hostCheckLabel.level = JobLogLevel::TASK_LOG_ERROR;
        hostCheckLabel.label = NUTANIX_HOST_CHECK_FAILED_LABEL;
        hostCheckLabel.params = std::vector<std::string>{ hostId, HOST_STATUS_MAP[statusFlag] };
        ReportJobDetail(hostCheckLabel);
        return false;
    }

    // 当前Element单机场景下架构集群内一致，不用检查目标位置架构类型
    return true;
}

int32_t NutanixProtectEngine::GetNewVMMachineName(std::string &vmName)
{
    Json::Value jobAdvancePara;
    if (m_restorePara == nullptr) {
        ERRLOG("Convert m_restorePara nullptr failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (!m_restoreExtendInfo.vmName.empty()) {
        vmName = m_restoreExtendInfo.vmName;
    }
    if (vmName.empty()) {
        ERRLOG("VM machine name provided is empty. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Get vm machine name: %s", vmName.c_str());
    return SUCCESS;
}

bool NutanixProtectEngine::CheckVMNameValidity(const std::string &vmName)
{
    // Nutanix6.5版本有bug，点和加号同时存在时，无法通过VM Name正常过滤出虚拟机;
    // Restricted characters include ", ', *, ,, [, ], ? and a protected VM name should not be longer
    // than 80 characters.
    static const std::regex e(RESTRICTED_CHARACTERS);
    if (vmName.empty() || Utils::GetUtf8CharNumber(vmName) > VM_NAME_LIMIT ||
        !std::regex_match(vmName, e)) {
        ERRLOG("Invalid vm name: %s", vmName.c_str());
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::CheckVMNameUnique(const std::string &vmName)
{
    INFOLOG("Enter");
    if (m_restoreLevel == RestoreLevel::RESTORE_TYPE_VM) {
        ApplicationLabelType labelParam;
        labelParam.label = NUTANIX_VMNAME_CHECK_LABEL;
        labelParam.params = std::vector<std::string>{ vmName };
        ReportJobDetail(labelParam);

        // 全字匹配;
        GetVMListRequest checkVmNameReq(0, 1, vmName);
        SetCommonInfo(checkVmNameReq);
        std::shared_ptr<NutanixResponse<struct VMListDataResponse>> response =
            m_nutanixClient->ExecuteAPI<NutanixResponse<struct VMListDataResponse>, GetVMListRequest>(checkVmNameReq);
        if (response == nullptr) {
            ERRLOG("Check vm unique name failed.");
            return FAILED;
        }
        if (response->GetResult().entities.size() > 0) {
            ERRLOG("New vm name %s exists.", response->GetResult().entities[0].name.c_str());
            ApplicationLabelType labelParam;
            labelParam.level = JobLogLevel::TASK_LOG_ERROR;
            labelParam.label = NUTANIX_VMNAME_NOTUNIQUE_LABEL;
            labelParam.params = std::vector<std::string>{ vmName };
            labelParam.errCode = CNWARE_VMNAME_CHECK_ERROR;
            ReportJobDetail(labelParam);
            return FAILED;
        }
    }
    INFOLOG("Check vm name unique success. %s", m_jobId.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::InitParaAndGetTargetVolume(const ApplicationResource &targetVol,
    struct RestorTargetVolume &targetVolume)
{
    Json::Value volExtendInfo;
    // 解析extend信息;
    INFOLOG("Target volume(%s) extend info: %s", targetVol.id.c_str(), targetVol.extendInfo.c_str());
    if (!Module::JsonHelper::JsonStringToJsonValue(targetVol.extendInfo, volExtendInfo)) {
        ERRLOG("JsonStringToJsonValue failed. targetVol's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    // 解析目标卷信息;
    if (!volExtendInfo.isMember("targetVolume")) {
        ERRLOG("No targetVolume provided. %s", m_jobId.c_str());
        return FAILED;
    }
    std::string targetVolumeStr = volExtendInfo["targetVolume"].asString();
    if (!Module::JsonHelper::JsonStringToStruct(targetVolumeStr, targetVolume)) {
        ERRLOG("JsonStringToJsonValue failed. targetVol's extendInfo, %s", m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Init jobPara and repoHandle success, get targetVolume success");
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckTargetVolumeDatastoreParam(const struct RestorTargetVolume &targetVolume)
{
    // 整机恢复和新键盘恢复，任务参数都会带datastore信息;
    // 使用已有磁盘进行磁盘恢复，任务参数不带datastore信息;
    INFOLOG("Enter");
    if (targetVolume.datastore.m_name.empty() || targetVolume.datastore.m_moRef.empty()) {
        WARNLOG("No datastore info provided. %s", m_jobId.c_str());
        return FAILED;
    }
    INFOLOG("Get datastore from request parameter(targetVolume) success.");
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetTargetContainerList(const ApplicationResource &targetVol,
    std::map<std::string, std::vector<std::string>> &containerList)
{
    struct RestorTargetVolume targetVolume;
    INFOLOG("Enter");
    if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
        ERRLOG("Get target volume info failed, %s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("Restore level %d", static_cast<int32_t>(m_restoreLevel));
    if (CheckTargetVolumeDatastoreParam(targetVolume) != SUCCESS) {
        ERRLOG("CheckTargetVolumeDatastoreParam failed, %s", m_jobId.c_str());
        return FAILED;
    }
    std::map<std::string, std::vector<std::string>>::iterator it = containerList.find(targetVolume.datastore.m_moRef);
    if (it != containerList.end()) {
        it->second.push_back(targetVol.name);
    } else {
        containerList[targetVolume.datastore.m_moRef] = std::vector<std::string>{ targetVol.name };
    }
    INFOLOG("Get target pool id list success.");
    return SUCCESS;
}

bool NutanixProtectEngine::GetStorageName(const std::string &containerId, std::string &containerName)
{
    INFOLOG("Enter");
    NutanixStorageContainerInfo storageContainerInfo;
    if (!GetContainerInfo(containerId, storageContainerInfo)) {
        ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
        return false;
    }
    containerName = storageContainerInfo.name;
    return true;
}

int32_t NutanixProtectEngine::QueryAgentHostClusterUUID(std::string &clusterUUID, std::string &agentHostName)
{
    INFOLOG("Enter");
    std::string agentDomainId = Utils::GetProxyHostId(true);
    if (agentDomainId.empty()) {
        ERRLOG("Get agentDomainId failed, %s", m_jobId.c_str());
        return FAILED;
    }
    NutanixVMInfo agentVmInfo;
    if (!GetVMInfoById(agentDomainId, agentVmInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_jobId.c_str());
        return FAILED;
    }

    GetHostInfoRequest req(agentVmInfo.hostId);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct HostInfo>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct HostInfo>, GetHostInfoRequest>(req);
    if (response == nullptr) {
        ERRLOG("GetHostInfoResponse failed! host id: %s", agentVmInfo.hostId.c_str());
        return false;
    }
    clusterUUID = response->GetResult().clusterId;
    agentHostName = response->GetResult().name;
    INFOLOG("QueryAgentHostCluster success.");
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckStoragePoolRestore(const std::vector<ApplicationResource> &restoreSubObjects)
{
    // 获取-目标存储池ID(列表), <volName, containerId>;
    // 获取container列表, key:container id， value：array of VMdisk UUID ;
    std::map<std::string, std::vector<std::string>> containerList;
    for (auto restoreVol : restoreSubObjects) {
        std::transform(restoreVol.id.begin(), restoreVol.id.end(), restoreVol.id.begin(), ::tolower);
        if (GetTargetContainerList(restoreVol, containerList) != SUCCESS) {
            ERRLOG("Get target pool id list failed.");
            return FAILED;
        }
    }
    std::string agentClusterUUID;
    std::string agentHostName;
    if (QueryAgentHostClusterUUID(agentClusterUUID, agentHostName) != SUCCESS) {
        ERRLOG("Get agent cluster id failed.");
        return FAILED;
    }
    // 查询代理主机是否联通目标存储池;
    std::string storeLists;
    std::vector<std::string> noTasksArgs{};
    int32_t iRet = SUCCESS;
    for (auto it : containerList) {
        std::string containerName;
        NutanixStorageContainerInfo storageContainerInfo;
        if (!GetContainerInfo(it.first, storageContainerInfo)) {
            ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
            return FAILED;
        }
        ApplicationLabelType labelParam;
        labelParam.label = NUTANIX_STORAGEPOOL_CHECK_LABEL;
        labelParam.params = std::vector<std::string>{it.second[0], storageContainerInfo.name};
        ReportJobDetail(labelParam);
        if (storageContainerInfo.markRemoval == true || storageContainerInfo.usageStats.userFreeBytes == 0 ||
            storageContainerInfo.clusterId != agentClusterUUID) {
            // 容器被删除或者没有可用容量了、或者备份代理虚拟机的clusterID与容器的不一样，会导致备份代理无法访问;
            ERRLOG("Check storage pool connection failed, volume: %s, pool: %s, %s",
                it.second[0].c_str(), it.first.c_str(), m_jobId.c_str());
            storeLists = storeLists + storageContainerInfo.name + ",";
            iRet = FAILED;
        }
    }
    if (!storeLists.empty()) {
        storeLists.pop_back();
        noTasksArgs = std::vector<std::string>{ agentHostName, storeLists};
        SetNoTasksArgs(noTasksArgs);
        return FAILED;
    }
    INFOLOG("Check storage pool for restore success.");
    return iRet;
}

bool NutanixProtectEngine::CheckCpuandMemoryLimit(NutanixVMInfo &domainInfo)
{
    INFOLOG("Enter CheckCpuandMemoryLimit");
    if (m_nutanixClient == nullptr) {
        ERRLOG("CheckDiskType para nullptr! %s", m_jobId.c_str());
        return false;
    }

    GetHostListRequest req(1, 1);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct HostListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct HostListDataResponse>, GetHostListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    const HostListDataResponse &hostList = resp->GetResult();

    if (hostList.entities.size() == 0) {
        WARNLOG("No host founded!");
        return false;
    }

    HostListResponse host = hostList.entities[0];
    int32_t hostMaxVcpu = host.numCpuCores * host.numCpuSockets * host.numCpuThreads; //  计算单个主机的CPU核数
    if (domainInfo.numVcpus * domainInfo.numCoresPerVcpu > hostMaxVcpu) {
        ERRLOG("cpu of host(%s) is not enough! numCpuCores:%d, numcoresPerSocket:%d, numCpuThreads:%d.",
            host.name.c_str(), host.numCpuCores, host.numCpuSockets, host.numCpuThreads);
        return false;
    }
    
    if (static_cast<uint64_t>(domainInfo.memoryMb) * NUTANIX_MB_TO_BYTE > host.memorySize) {
        ERRLOG("Memory of host(%s) is not enough! m_memoryMb:%d, memoryCapacity:%llu.",
            host.name.c_str(), domainInfo.memoryMb, host.memorySize);
        return false;
    }
    return true;
}

int32_t NutanixProtectEngine::CheckBeforeRecover(const VMInfo &vmObj)
{
    INFOLOG("Enter");
    if (m_restorePara == nullptr && !InitJobPara()) {
        ERRLOG("InitJobPara, %s", m_taskId.c_str());
        return FAILED;
    }
    ApplicationLabelType labelParam;
    labelParam.label = NUTANIX_RESTORE_PRECHECK_LABEL;
    ReportJobDetail(labelParam);

    struct NutanixVMInfo domainInfo;
    std::string metadata = vmObj.m_metadata;
    // 获取原虚拟机信息;
    if (!Module::JsonHelper::JsonStringToStruct(metadata, domainInfo)) {
        ERRLOG("Get new domain info failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckHostBeforeRecover(vmObj)) {
        ERRLOG("Check host before recover failed. %s", m_jobId.c_str());
        return FAILED;
    }

    std::string vmName;
    GetNewVMMachineName(vmName);
    if (!CheckVMNameValidity(vmName)) {
        ERRLOG("Check vm name failed. %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CheckCpuandMemoryLimit(domainInfo)) {
        ERRLOG("Check Vm vcpu and memory limit failed. %s", m_jobId.c_str());
        return FAILED;
    }

    if (!CheckVMStatus()) {
        ERRLOG("Check VM Status Failed, %s", m_taskId.c_str());
        return FAILED;
    }
    if (CheckStoragePoolRestore(m_restorePara->restoreSubObjects) != SUCCESS) {
        ERRLOG("Check storage pool failed.");
        ApplicationLabelType labelParam;
        labelParam.level = JobLogLevel::TASK_LOG_ERROR;
        labelParam.label = CNWARE_STORAGEPOOL_FAILED_LABEL;
        ReportJobDetail(labelParam);
        return FAILED;
    }

    INFOLOG("CheckBeforeRecover success.");
    return SUCCESS;
}

bool NutanixProtectEngine::CheckVolPreSnapshotValidity(
    const VmDiskInfo &currVol, const VolSnapInfo &snapshotVol)
{
    VolInfo diskInfo;
    if (!Module::JsonHelper::JsonStringToStruct(snapshotVol.m_metadata, diskInfo)) {
        ERRLOG("Transfer DomainDiskDevicesResp failed, %s", m_taskId.c_str());
        return false;
    }
    INFOLOG("Enter, currVol.size(%llu) snapshotVol.m_size(%llu)", currVol.size, diskInfo.m_volSizeInBytes);
    if (currVol.size > diskInfo.m_volSizeInBytes) {
        WARNLOG("Target vol(%s) capacity increased", currVol.diskAddress.vmdiskUuid.c_str());
        return false;
    }
    return true;
}

bool NutanixProtectEngine::CheckPreSnapShotValidity(
    const AppProtect::BackupJob &job, const std::vector<VolSnapInfo> &lastVolList)
{
    INFOLOG("Enter Nutanix CheckPreSnapShotValidity");
    NutanixVMInfo tmp;
    if (!Module::JsonHelper::JsonStringToStruct(m_vmInfo.m_metadata, tmp)) {
        ERRLOG("Failed to get VMinfo");
        return false;
    }
    std::vector<VmDiskInfo> targetVolsInfo = tmp.vmDiskInfo;
    std::unordered_map<std::string, uint64_t> mapLastVol;
    for (int32_t idx = 0; idx < lastVolList.size(); ++idx) {
        mapLastVol[lastVolList[idx].m_volUuid] = idx;
    }

    for (const auto &targetVol : targetVolsInfo) {
        if (targetVol.isCdrom) {
            continue;
        }
        auto itMap = mapLastVol.find(targetVol.diskAddress.vmdiskUuid);
        if (itMap == mapLastVol.end() || !CheckVolPreSnapshotValidity(targetVol, lastVolList[itMap->second])) {
            WARNLOG("Check vol (%s) previous snapshot validity failed. task id: %s",
                targetVol.diskAddress.vmdiskUuid.c_str(), m_taskId.c_str());
            INFOLOG("targe vol id:%s, size: %llu", targetVol.diskAddress.vmdiskUuid.c_str(), targetVol.size);
            return false;
        }
    }
    return true;
}

int32_t NutanixProtectEngine::CheckBackupJobType(const VirtPlugin::JobTypeParam& jobTypeParam, bool& checkRet)
{
    INFOLOG("Enter Nutanix checkbackupjobtype");
    if (ConnectToEnv(jobTypeParam.m_job.protectEnv) != SUCCESS) {
        ERRLOG("Failed to connect protect environment.");
        return FAILED;
    }

    if (!DoGetMachineMetadata(jobTypeParam.m_job.protectObject.id)) {
        ERRLOG("Set VM metadata info failed, %s", m_taskId.c_str());
        return FAILED;
    }

    if (m_vmInfo.m_volList.empty()) {
        ERRLOG("Fail to get vm volume list is empty.task id: %s", m_taskId.c_str());
        return FAILED;
    }

    if (!CheckPreSnapShotValidity(jobTypeParam.m_job, jobTypeParam.m_snapshotInfo.m_volSnapList)) {
        WARNLOG("Check last snapshot info validity failed, convert to full backup.task id: %s",
            m_taskId.c_str());
        checkRet = false;
        return SUCCESS;
    }
    checkRet = true;
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckBeforeMount()
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckBeforeUnmount()
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::CheckResourceUsage(const std::string &resourceId)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::CreateVolume(const VolInfo &backupVol, const std::string &volMetaData,
    const std::string &vmMoRef, const DatastoreInfo &storage, VolInfo &newVol)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::CancelLiveMount(const VMInfo &liveVm)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm)
{
    return SUCCESS;
}

int32_t NutanixProtectEngine::MigrateLiveVolume(const VMInfo &liveVm)
{
    return SUCCESS;
}

void NutanixProtectEngine::InitRepoHandler(void)
{
    if (m_metaRepoHandler != nullptr && m_cacheRepoHandler != nullptr) {
        return;
    }
    std::vector<AppProtect::StorageRepository> repos = m_jobHandle->GetStorageRepos();
    for (const auto &repo : repos) {
        if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
            m_metaRepoPath = repo.path[0];
        } else if (repo.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
            m_cacheRepoPath = repo.path[0];
        }
    }
}

int32_t NutanixProtectEngine::LoadCopyVolumeMatadata(const std::string &volId, VolInfo &volInfo)
{
    DBGLOG("RestoreMode: %s", m_restorePara->jobParam.restoreMode.c_str());
    std::shared_ptr<RepositoryHandler> repositoryHandle =
        (m_metaRepoHandler != nullptr) ? m_metaRepoHandler : m_cacheRepoHandler;
    std::string repoPath = (m_metaRepoHandler != nullptr) ? m_metaRepoPath : m_cacheRepoPath;

    std::string volmetaDataPath = repoPath + VIRT_PLUGIN_VOLUMES_META_DIR + volId + ".ovf";
    if (Utils::LoadFileToStructWithRetry(repositoryHandle, volmetaDataPath, volInfo) != SUCCESS) {
        ERRLOG("Failed to load file %s, %s", volmetaDataPath.c_str(), m_taskId.c_str());
        return FAILED;
    }
    INFOLOG("Load file %s success.", volmetaDataPath.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::CreateDiskByCopy(const VolInfo &copyVol, VolInfo &dstVolInfo, DatastoreInfo &storage)
{
    // 由于Nutanix的特殊性，不能在主任务的流程创卷;
    return SUCCESS;
}

int32_t NutanixProtectEngine::GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
    VolMatchPairInfo &volPairs)
{
    InitRepoHandler();
    VolInfo dstVolInfo;
    struct RestorTargetVolume targetVolume;
    if (InitParaAndGetTargetVolume(targetVol, targetVolume) != SUCCESS) {
        ERRLOG("Get target volume info failed.");
        return FAILED;
    }
    DatastoreInfo storage;
    if (CreateDiskByCopy(copyVol, dstVolInfo, targetVolume.datastore) != SUCCESS) {
        ERRLOG("Create disk by copy failed.");
        return FAILED;
    }

    dstVolInfo.m_name = copyVol.m_name;
    dstVolInfo.m_slotId = copyVol.m_slotId;
    dstVolInfo.m_type = copyVol.m_type == "" ? BUS_TYPE_STR_SCSI : copyVol.m_type;
    dstVolInfo.m_volSizeInBytes = copyVol.m_volSizeInBytes;
    dstVolInfo.m_datastore.m_name = targetVolume.datastore.m_name;
    dstVolInfo.m_datastore.m_moRef = targetVolume.datastore.m_moRef;
    volPairs.AddVolPair(copyVol, dstVolInfo);
    return SUCCESS;
}

void NutanixProtectEngine::DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType)
{
    return;
}

void NutanixProtectEngine::CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
    const AppProtect::Application &application)
{
    INFOLOG("CheckApplication start! Task id:%s", m_jobId.c_str());
    SetAppEnv(appEnv);
    std::vector<std::string> params {};
    if (!Init()) {
        ERRLOG("Init nutanix engine failed! Taskid: %s", appEnv.id.c_str());
        params.emplace_back("");
        returnValue.__set_bodyErr(NUTANIX_ERR_PARAM);
        returnValue.__set_bodyErrParams(params);
        returnValue.__set_code(ACTION_ERROR);
        return;
    }
    GetClusterListRequest req(0, 1);
    SetCommonInfo(req);
    std::string errorDes;
    int64_t errorCode(0);
    if (m_nutanixClient->CheckAuth(req, errorCode, errorDes) != SUCCESS) {
        auto checkApplicationError = nutanixErrorCodeMap.count(errorCode) == 0
        ? NUTANIX_CONNECT_FAILED : nutanixErrorCodeMap[errorCode];
        returnValue.__set_bodyErr(checkApplicationError);
        params.emplace_back("");
        returnValue.__set_bodyErrParams(params);
        returnValue.__set_code(ACTION_ERROR);
        return;
    }

    INFOLOG("CheckApplication success! Task id:%s", m_jobId.c_str());
    params.emplace_back("");
    returnValue.__set_bodyErrParams(params);
    returnValue.__set_code(ACTION_SUCCESS);
    return;
}

void NutanixProtectEngine::ListApplicationResource(std::vector<ApplicationResource>& returnValue,
    const ApplicationEnvironment& appEnv, const Application& application, const ApplicationResource& parentResource)
{
    return;
}

void NutanixProtectEngine::DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv)
{
    return;
}


void NutanixProtectEngine::DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
    const Application& application)
{
    SetAppEnv(appEnv);
    int64_t errorCode(0);
    if (!Init()) {
        ERRLOG("Init nutanix engine failed! Taskid: %s", appEnv.id.c_str());
        return;
    }
    std::string errorDes;
    if (!InitClient(errorCode, errorDes)) {
        ERRLOG("InitClient failed, errorcode= %d, Des: %s! Taskid: %s", errorCode,
            errorDes, appEnv.id.c_str());
        return;
    }

    GetClusterListRequest req(0, 1);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct ClusterListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct ClusterListDataResponse>, GetClusterListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return;
    }
    ClusterListDataResponse clusterList = resp->GetResult();

    std::vector<ApplicationResource> resourceResults;

    if (clusterList.entities.size() == 0) {
        ERRLOG("No cluster founded!");
    }

    Json::Value body;
    body["productVersion"] = clusterList.entities[0].version;
    Json::FastWriter writer;
    returnEnv.__set_extendInfo(writer.write(body));

    returnEnv.__set_id(clusterList.entities[0].uuid);
    returnEnv.__set_name(clusterList.entities[0].name);
    returnEnv.__set_type(appEnv.type);
    returnEnv.__set_subType(appEnv.subType);
    returnEnv.__set_endpoint(appEnv.endpoint);
    return;
}

bool NutanixProtectEngine::IfDeleteLatestSnapShot()
{
    InitRepoHandler();
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("m_metaRepoHandler is nullptr, RepositoryHandler init failed.");
        return false;
    }
    std::string file = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (!m_metaRepoHandler->Exists(file)) {
        INFOLOG("Snapshot.info file(%s) does not exist, %s", file.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Begin to query volume from file(%s).", file.c_str());
    SnapshotInfo tmpSnapInfo;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, file, tmpSnapInfo) != SUCCESS) {
        ERRLOG("Load context failed, %s", m_taskId.c_str());
        return false;
    }
    if (DeleteSnapshot(tmpSnapInfo) != SUCCESS) {
        WARNLOG("Delete snapshot failed, vm: %s.", tmpSnapInfo.m_vmMoRef.c_str());
    }
    return true;
}

int32_t NutanixProtectEngine::RestoreVolMetadata(VolMatchPairInfo &volPairs, const VMInfo &vmInfo)
{
    return SUCCESS;
}

bool NutanixProtectEngine::GetVMInfoById(const std::string &vmId, NutanixVMInfo &nutanixVmInfo)
{
    INFOLOG("Enter Nutanix GetVMInfoById, id: %s", vmId.c_str());
    GetVMInfoRequest req(vmId);
    SetCommonInfo(req);
    if (m_nutanixClient == nullptr) {
        ERRLOG("Get VM info m_nutanixClient nullptr, %s", m_taskId.c_str());
        return false;
    }
    std::shared_ptr<NutanixResponse<struct NutanixVMInfo>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixVMInfo>, GetVMInfoRequest>(req);
    if (resp == nullptr) {
        ERRLOG("Get VM info failed, %s", m_taskId.c_str());
        return false;
    }
    nutanixVmInfo = std::move(resp->GetResult());
    return true;
}

int32_t NutanixProtectEngine::GetClusterArch(ClusterListResponse &clusterInfo)
{
    GetClusterListRequest req(0, 1);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct ClusterListDataResponse>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct ClusterListDataResponse>, GetClusterListRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetResource failed! Ip: %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    ClusterListDataResponse clusterList = resp->GetResult();
    if (clusterList.entities.size() == 0) {
        ERRLOG("clusterList is empty. : %s", req.GetEnvAddress().c_str());
        return FAILED;
    }
    clusterInfo = clusterList.entities.front();
    INFOLOG("Target cluster %s arch: %s.", clusterInfo.uuid.c_str(), clusterInfo.arch.c_str());
    return SUCCESS;
}

int32_t NutanixProtectEngine::GetNetworkById(const std::string &networkId, NetworkInfo &networkInfo)
{
    GetNetworkByIdRequest req(networkId);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct NetworkInfo>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NetworkInfo>, GetNetworkByIdRequest>(req);
    if (resp == nullptr) {
        ERRLOG("GetNetwork failed! Id: %s", networkId.c_str());
        return FAILED;
    }
    networkInfo = resp->GetResult();
    return SUCCESS;
}

int32_t NutanixProtectEngine::SetVmNetwork(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo)
{
    for (const auto &inter : nutanixVmInfo.vmNics) {
        NetworkInfo networkInfo;
        if (GetNetworkById(inter.networkUuid, networkInfo) != SUCCESS) {
            ERRLOG("GetNetwork failed!");
            return FAILED;
        }
        BridgeInterfaceInfo interface;
        interface.m_uuid = inter.nicUuid;
        interface.m_name = networkInfo.name;
        interface.m_mac = inter.macAddress;
        interface.m_moRef = vmInfo.m_uuid;
        interface.m_ip = inter.ipAddress;
        VmNic tmpInter = inter;
        if (!Module::JsonHelper::StructToJsonString(tmpInter, interface.m_metadata)) {
            ERRLOG("Convert Nutanix Vic Info to json string failed, %s", m_taskId.c_str());
            return FAILED;
        }
        vmInfo.m_interfaceList.emplace_back(interface);
    }
    return SUCCESS;
}

int32_t NutanixProtectEngine::SetNutanixVMInfo2VMInfo(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo)
{
    INFOLOG("Enter Nutanix SetNutanixVMInfo2VMInfo");
    vmInfo.m_moRef = nutanixVmInfo.id;
    vmInfo.m_uuid = nutanixVmInfo.id;
    vmInfo.m_name = nutanixVmInfo.name;
    vmInfo.m_location = nutanixVmInfo.hostId;
    vmInfo.m_locationName = nutanixVmInfo.hostId;

    if (SetVmNetwork(nutanixVmInfo, vmInfo) != SUCCESS) {
        ERRLOG("SetVmNetwork failed, %s", m_taskId.c_str());
        return FAILED;
    }

    NutanixVMInfo tmpVmInfo = nutanixVmInfo;
    if (!Module::JsonHelper::StructToJsonString(tmpVmInfo, vmInfo.m_metadata)) {
        ERRLOG("Convert NutanixVMInfo to json string failed, %s", m_taskId.c_str());
        return FAILED;
    }
    NutanixVMMetaDataStruct VMMetaInfo;
    VMMetaInfo.VmInfo = nutanixVmInfo;
    if (GetClusterArch(VMMetaInfo.clusterInfo) != SUCCESS) {
        ERRLOG("Get cluster arch failed, %s",  m_taskId.c_str());
        return FAILED;
    }
    if (!Module::JsonHelper::StructToJsonString(VMMetaInfo, vmInfo.m_extendInfo)) {
        ERRLOG("Convert NutanixVMInfo to json string failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool NutanixProtectEngine::GetContainerInfo(const std::string &storageContainerId,
    NutanixStorageContainerInfo &storageContainerInfo)
{
    INFOLOG("Enter Nutanix Engine GetContainerInfo");
    GetStorageContainerRequest req(storageContainerId);
    SetCommonInfo(req);
    if (m_nutanixClient == nullptr) {
        ERRLOG("Get StorageContainer info m_nutanixClient nullptr, %s", m_taskId.c_str());
        return false;
    }
    std::shared_ptr<NutanixResponse<struct NutanixStorageContainerInfo>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixStorageContainerInfo>, GetStorageContainerRequest>(req);
    if (response == nullptr) {
        ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
        return false;
    }
    storageContainerInfo = response->GetResult();
    return true;
}

bool NutanixProtectEngine::SetVolListInfo2VMInfo(const NutanixVMInfo &nutanixVmInfo, VMInfo &vmInfo)
{
    INFOLOG("Enter Nutanix SetVolListInfo2VMInfo");
    for (const auto &disk : nutanixVmInfo.vmDiskInfo) {
        if (disk.isCdrom || !disk.diskAddress.volumeGroupUuid.empty()) {
            continue;
        }
        VolInfo volInfo;
        volInfo.m_name = disk.diskAddress.diskLabel;
        volInfo.m_moRef = disk.diskAddress.vmdiskUuid;
        volInfo.m_uuid = disk.diskAddress.vmdiskUuid;
        volInfo.m_type = disk.diskAddress.deviceBus;
        volInfo.m_volSizeInBytes = disk.size;
        volInfo.m_bootable = std::to_string(disk.isCdrom);
        volInfo.m_slotId = std::to_string(disk.diskAddress.deviceIndex);
        volInfo.m_vmMoRef = vmInfo.m_uuid;
        volInfo.m_datastore.m_moRef = disk.storageContainerUuid;
        volInfo.m_location = disk.diskAddress.ndfsFilepath;
        NutanixStorageContainerInfo storageContainerInfo;
        if (!GetContainerInfo(disk.storageContainerUuid, storageContainerInfo)) {
            ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
            return false;
        }
        volInfo.m_datastore.m_poolId = storageContainerInfo.containerId;
        volInfo.m_datastore.m_name = storageContainerInfo.name;
        if (!Module::JsonHelper::StructToJsonString(storageContainerInfo, volInfo.m_datastore.m_extendInfo)) {
            ERRLOG("Convert Nutanix storageContainerInfo to json string failed, %s", m_taskId.c_str());
            return false;
        }
        VmDiskInfo tmp = disk;
        if (!Module::JsonHelper::StructToJsonString(tmp, volInfo.m_metadata)) {
            ERRLOG("Convert Nutanix DiskInfo to json string failed, %s", m_taskId.c_str());
            return false;
        }
        DBGLOG("Trans disk info to VolInfo success.%s %s %s Volume size: %llu  volume type: %s. %s.",
               volInfo.m_uuid.c_str(), volInfo.m_name.c_str(), volInfo.m_type.c_str(), volInfo.m_volSizeInBytes,
               volInfo.m_volumeType.c_str(), m_taskId.c_str());
        vmInfo.m_volList.emplace_back(volInfo);

        INFOLOG("Set VMInfo volInfo.m_uuid: %s", volInfo.m_uuid.c_str());
    }
    return true;
}

bool NutanixProtectEngine::DoGetMachineMetadata(const std::string &vmId)
{
    INFOLOG("Enter Nutanix DoGetMachineMetadata, id %s", vmId.c_str());
    if (m_machineMetaCached) {
        INFOLOG("Nutanix Machine metadata cached, %s", m_taskId.c_str());
        return true;
    }

    NutanixVMInfo nutanixVmInfo;
    if (!GetVMInfoById(vmId, nutanixVmInfo)) {
        ERRLOG("Get VM info failed when get meta data, %s", m_taskId.c_str());
        return false;
    }

    VMInfo protectVmInfo;
    if (SetNutanixVMInfo2VMInfo(nutanixVmInfo, protectVmInfo) != SUCCESS) {
        ERRLOG("Get VM Cluster or nic info failed, %s", m_taskId.c_str());
        return false;
    }

    if (!SetVolListInfo2VMInfo(nutanixVmInfo, protectVmInfo)) {
        ERRLOG("Get VM vol list info failed, %s", m_taskId.c_str());
        return false;
    }

    m_vmInfo = protectVmInfo;
    m_machineMetaCached = true;
    DBGLOG("Do get machine metadata success, %s", m_taskId.c_str());
    return true;
}

bool NutanixProtectEngine::CheckTaskStatus(const std::string &taskId)
{
    INFOLOG("Enter Nutanix CheckTaskStatus, taskid %s", taskId.c_str());
    QueryTaskRequest req(taskId);
    SetCommonInfo(req);
    std::string taskStatus = NUTANIX_TASK_STATUS_NONE;
    std::shared_ptr<NutanixResponse<struct NutanixTaskResponse>> response;
    uint32_t times = 0;
    while (times * COMMON_WAIT_TIME_3S <NUTANIX_MAX_TIME_OUT) {
        times++;
        response = m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixTaskResponse>, QueryTaskRequest>(req);
        if (response == nullptr) {
            ERRLOG("Get task info failed, %s", taskId.c_str());
            return false;
        }
        taskStatus = response->GetResult().status;
        INFOLOG("taskStatus : %s", taskStatus.c_str());
        if (taskStatus != NUTANIX_TASK_STATUS_RUNNING && taskStatus != NUTANIX_TASK_STATUS_QUEUED) {
            break;
        } else {
            sleep(COMMON_WAIT_TIME_3S);
        }
    }
    if (taskStatus != NUTANIX_TASK_STATUS_SUCCEEDED) {
        ERRLOG("Nutanix taskId %s failed, status %s, stepDesc %s.", taskId.c_str(),
            taskStatus.c_str(), (response->GetResult().errMsg.errorDetail).c_str());
        return false;
    }
    return true;
}

bool NutanixProtectEngine::SetVolSnapInfo(const VmDisk &disk, VolSnapInfo &volSnap,
    const NutanixSnapshotInfo &nutanixSnapShot)
{
    INFOLOG("Enter Nutanix SetVolSnapInfo");
    if (disk.vmDiskClone.diskAddress.vmdiskUuid.empty()) {
        return false;
    }
    volSnap.m_volUuid = disk.vmDiskClone.diskAddress.vmdiskUuid;
    volSnap.m_snapshotId = nutanixSnapShot.groupId;
    volSnap.m_snapshotName = nutanixSnapShot.name;
    VolInfo tmpVol;
    for (const auto &diskinfo : m_vmInfo.m_volList) {
        if (diskinfo.m_uuid == volSnap.m_volUuid) {
            tmpVol = diskinfo;
            volSnap.m_size = diskinfo.m_volSizeInBytes;
        }
    }
    VmDisk tmpdisk = disk;
    if (!Module::JsonHelper::StructToJsonString(tmpdisk, volSnap.m_extendInfo)) {
        ERRLOG("Convert SnapshotDiskInfo to json string failed, %s", m_taskId.c_str());
        return false;
    }
    INFOLOG("VolSnapshotInfo is : %s", volSnap.m_extendInfo.c_str());
    if (!Module::JsonHelper::StructToJsonString(tmpVol, volSnap.m_metadata)) {
        ERRLOG("Convert DomainDiskDevicesResp to json string failed, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

bool NutanixProtectEngine::GetSnapshotResult(const std::string &snapShotTaskId, const std::string &snapshotId,
    SnapshotInfo &snapshot, std::string &errCode)
{
    DBGLOG("Start to check create snapshots result. %s", m_taskId.c_str());
    INFOLOG("Enter nutanix GetSnapshotResult");
    if (!CheckTaskStatus(snapShotTaskId)) {
        ERRLOG("Create snapshot task %s failed, %s", snapShotTaskId.c_str(), m_taskId.c_str());
        return false;
    }
    INFOLOG("Enter nutanix GetSnapshot");
    GetSnapshotRequest req(snapshotId);
    SetCommonInfo(req);
    std::shared_ptr<NutanixResponse<struct NutanixSnapshotInfo>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixSnapshotInfo>, GetSnapshotRequest>(req);
    if (response == nullptr) {
        ERRLOG("GetSnapshot failed!");
        return false;
    }
    std::string logSnapshotinfo;
    NutanixSnapshotInfo lognutanixSnapShot = response->GetResult();
    if (!Module::JsonHelper::StructToJsonString(lognutanixSnapShot, logSnapshotinfo)) {
        ERRLOG("Convert SnapshotDiskInfo to json string failed, %s", m_taskId.c_str());
        return false;
    }
    DBGLOG("SnapshotInfo is : %s", logSnapshotinfo.c_str());
    
    for (auto &disk : response->GetResult().oringeVm.vmDisks) {
        VolSnapInfo volSnap;
        if (!SetVolSnapInfo(disk, volSnap, lognutanixSnapShot)) {
            ERRLOG("Get vol snap info failed, %s", m_taskId.c_str());
            continue;
        }
        snapshot.m_volSnapList.push_back(volSnap);
    }
    return true;
}

int32_t NutanixProtectEngine::ConnectToEnv(const AppProtect::ApplicationEnvironment &env)
{
    INFOLOG("Enter Nutanix ConnectToEnv");
    SetAppEnv(env);
    if (!Init()) {
        ERRLOG("Init Nutanix engine failed! %s", m_taskId.c_str());
        return FAILED;
    }
    int64_t errorCode(0);
    std::string errorDes;
    if (!InitClient(errorCode, errorDes)) {
        ERRLOG("InitNutanixClient failed! %s, error is %llu:%s", m_taskId.c_str(),
               errorCode, errorDes);
        return FAILED;
    }
    return SUCCESS;
}

bool NutanixProtectEngine::GetVirtualDiskInfo(const std::string &diskId, VirtualDiskInfo &backupDiskInfo)
{
    INFOLOG("Enter Nutanix Engine GetVirtualDiskInfo");
    GetVirtualDiskInfoRequest req(diskId);
    SetCommonInfo(req);
    if (m_nutanixClient == nullptr) {
        ERRLOG("Get StorageContainer info m_nutanixClient nullptr, %s", m_taskId.c_str());
        return false;
    }
    std::shared_ptr<NutanixResponse<struct VirtualDiskInfo>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct VirtualDiskInfo>, GetVirtualDiskInfoRequest>(req);
    if (response == nullptr) {
        ERRLOG("Get StorageContainer info failed, %s", m_taskId.c_str());
        return false;
    }
    backupDiskInfo = response->GetResult();
    return true;
}
}