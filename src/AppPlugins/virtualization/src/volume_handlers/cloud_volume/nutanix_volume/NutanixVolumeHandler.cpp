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
#include "NutanixVolumeHandler.h"
#include <boost/uuid/uuid_io.hpp>
#include "common/Structs.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "curl_http/HttpStatus.h"

using namespace NutanixPlugin::NutanixErrorCode;

namespace {
const std::string MODULE_NAME = "NutanixVolumeHandler";
const int32_t COMMON_WAIT_TIME = 10;
const int32_t COMMON_WAIT_TIME_3S = 3;
const int32_t SCAN_DISK_RETRY_TIMES = 3;
const int32_t FIFTEEN_MIN_IN_SEC = 900;
const int RETRY_TIMES = 3;
std::mutex m_fileMutex;
const int32_t NUTANIX_MAX_TIME_OUT = 720000; // 防呆设置最大200小时超时时间
const uint32_t RETRY_COUNT = 10;
const uint64_t GB_IN_BYTES = 1073741824;
constexpr auto MODULE = "NutanixVolumeHandler";

}

namespace NutanixPlugin {
uint64_t g_attachLimit = -1;
uint64_t g_versionAttachLimit = -1;
std::mutex g_attachCntMutex;
std::condition_variable g_attachCntCv;
std::mutex g_attachVolumeMutex;

std::string NutanixVolumeHandler::GetProxyHostId()
{
    return Utils::GetProxyHostId(true);
}

int32_t NutanixVolumeHandler::DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    return SUCCESS;
}

int32_t NutanixVolumeHandler::DoWaitVolumeStatus(const std::string &volId, const std::string &status,
    std::vector<std::string> intermediateState, uint32_t interval, uint32_t retryCount)
{
    return SUCCESS;
}

int32_t NutanixVolumeHandler::DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    return SUCCESS;
}

bool NutanixVolumeHandler::CheckAndDetachVolume(const std::string &volId)
{
    return true;
}

bool NutanixVolumeHandler::InitClient()
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
    if (m_nutanixClient->Init(m_appEnv) != SUCCESS) {
        ERRLOG("Init nutanix client failed failed! Taskid: %s", m_appEnv.id.c_str());
        return false;
    }
    return true;
}

void NutanixVolumeHandler::SetCommonInfo(NutanixRequest& req)
{
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

bool NutanixVolumeHandler::CheckTaskStatus(const std::string taskId)
{
    QueryTaskRequest req(taskId);
    SetCommonInfo(req);
    std::string taskStatus = NUTANIX_TASK_STATUS_NONE;
    std::shared_ptr<NutanixResponse<struct NutanixTaskResponse>> response;
    uint32_t times = 0;
    while (times * COMMON_WAIT_TIME_3S < NUTANIX_MAX_TIME_OUT) {
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

bool NutanixVolumeHandler::GetTargetBusIndex()
{
    m_targetBusIndex = -1;
    for (auto diskInfo : m_agentVmInfoBeforeAttach.vmDiskInfo) {
        if (diskInfo.diskAddress.deviceBus != BUS_TYPE_STR_SCSI) {
            continue;
        }
        m_targetBusIndex = m_targetBusIndex <= diskInfo.diskAddress.deviceIndex ?
            diskInfo.diskAddress.deviceIndex + 1 : m_targetBusIndex;
    }
    if (m_targetBusIndex < 0) {
        ERRLOG("Get target bus index failed, %s", m_taskId.c_str());
        return false;
    }
    return true;
}

void NutanixVolumeHandler::SetNewCreateVmDiskInfo(AttachCreateNewVmDisk &newDisk)
{
    newDisk.attachDiskAddress.deviceIndex = m_targetBusIndex;
    newDisk.attachDiskAddress.deviceBus = BUS_TYPE_STR_SCSI;
    newDisk.vmDiskCreate.size = m_volInfo.m_volSizeInBytes;
    newDisk.vmDiskCreate.storageContainerUuid = m_volInfo.m_datastore.m_moRef;
    return;
}

int32_t NutanixVolumeHandler::GetAgentIp(std::string &strAgentIp)
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string agentWorkPath = "/DataBackup/ProtectClient/ProtectClient-E";
    std::string cmd = "grep listen " + agentHomedir + agentWorkPath + "/nginx/conf/nginx.conf \
        | awk -F ' ' '{print $2}' | awk -F ':' '{print $1}'";
    vector<std::string> paramList;
    vector<std::string> cmdOutput;
    vector<std::string> errDesc;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, cmdOutput, errDesc);
    if (ret != 0 || cmdOutput.size() != 1) {
        ERRLOG("Get Agent IP failed.");
        Module::printCmdOutputInfo(MODULE, 0, cmdOutput, errDesc);
        ret = FAILED;
    } else {
        strAgentIp = cmdOutput[0];
        INFOLOG("Agent ip: %s", strAgentIp.c_str());
    }
    return ret;
}

int32_t NutanixVolumeHandler::DoAttachVolumeToHostInner()
{
    AttachDiskRequest req(m_serverId);
    SetCommonInfo(req);
    if (m_isBackup) {
        std::string agentIp;
        if (GetAgentIp(agentIp) != SUCCESS) {
            WARNLOG("run cmd to get agent ip failed");
        }
        std::string volSize = std::to_string(double(m_volInfo.m_volSizeInBytes) / GB_IN_BYTES);
        ApplicationLabelType labelParam(NUTANIX_REPORT_LABEL_START_BACKUP,
        std::vector<std::string>{agentIp, m_volInfo.m_uuid, volSize});
        ReportJobDetail(labelParam);
        VmDisk vol;
        SetAttachSnapshot(vol, m_newCreateVmDisk);
        req.SetSnapshotCloneVmDisk(vol);
    } else {
        if (!GetTargetBusIndex()) {
            ERRLOG("GetTargetBusIndex failed : %s", m_taskId.c_str());
            return FAILED;
        }
        AttachCreateNewVmDisk newCreateVmDisk;
        SetNewCreateVmDiskInfo(newCreateVmDisk);
        req.SetNewCreateVmDisk(newCreateVmDisk, m_serverId);
    }
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, AttachDiskRequest>(req);
    if (response == nullptr || !CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Attach volume to agent failed, %s", m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool NutanixVolumeHandler::GetAgentVmInfo(NutanixVMInfo &vmInfo)
{
    GetVMInfoRequest req(m_serverId);
    SetCommonInfo(req);
    req.SetVmNicConfig(false);
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
    vmInfo = std::move(resp->GetResult());
    return true;
}

int32_t NutanixVolumeHandler::AttachDiskAndGetDev()
{
    INFOLOG("enter nutanixvolhandler AttachDiskAndGetDev");
    std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
    if (!GetAgentVmInfo(m_agentVmInfoBeforeAttach)) {
        ERRLOG("GetAgentVmInfo failed");
        return FAILED;
    }
    if (DoAttachVolumeToHostInner() != SUCCESS) {
        if (GetAttachLimit() > 0) {
            DBGLOG("Attach number check: notify. %s", m_jobId.c_str());
            g_attachCntCv.notify_one();
        }
        return FAILED;
    }
    if (!GetNewAttachedDisk()) {
        ERRLOG("Scan disk failed.");
        return FAILED;
    }

    return SUCCESS;
}

bool NutanixVolumeHandler::GetNewAttachDev(VmDiskInfo diskInfo)
{
    INFOLOG("Enter Get Dev diskid: %s", diskInfo.diskAddress.vmdiskUuid.c_str());
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string uuid(diskInfo.diskAddress.vmdiskUuid);
    std::replace(uuid.begin(), uuid.end(), '-', '_');
    std::vector<Module::CmdParam> cmdParam {
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "get_diskpath_for_wwn",
        Module::CmdParam(Module::COMMON_PARAM, uuid)
    };
    // 挂盘成功后，扫盘重试3次，查找盘符，避免扫盘时盘未就绪
    int retryTimes = 0;
    while (retryTimes < SCAN_DISK_RETRY_TIMES) {
        ++retryTimes;
        if (Utils::CallAgentExecCmd(cmdParam, cmdOut) == 0) {
            break;
        }
        WARNLOG("The device is not found, path:%s. retry:%d", uuid.c_str(), retryTimes);
        sleep(COMMON_WAIT_TIME_3S);
    }

    if (!cmdOut.empty()) {
        std::string strLine = cmdOut[0];
        INFOLOG("scan dev result : %s", strLine.c_str());
        size_t nPos = strLine.find("../../");
        if (nPos != std::string::npos) {
            std::string dev = strLine.substr(nPos+6);
            m_diskDevicePath = "/dev/" + dev;
            INFOLOG("Dev : %s", m_diskDevicePath.c_str());
            return true;
        }
    }
    ERRLOG("get dev of disk(%s) falied", uuid.c_str());
    return false;
}

bool NutanixVolumeHandler::FindAttachedDiskByNdfsPath(const NutanixVMInfo &agentVmiAfterAttach)
{
    for (auto diskInfo : agentVmiAfterAttach.vmDiskInfo) {
        if (m_ndfsFilePath == diskInfo.sourceDiskAddress.ndfsFilepath) {
            m_attachedDisk = diskInfo;
            m_newCreateVolId = diskInfo.diskAddress.vmdiskUuid;
            INFOLOG("Get attached snapshot disk");
            return true;
        }
    }
    return false;
}

bool NutanixVolumeHandler::FindAttachedDiskByTargetBusIndex(const NutanixVMInfo &agentVmiAfterAttach)
{
    for (auto diskInfo : agentVmiAfterAttach.vmDiskInfo) {
        if (diskInfo.diskAddress.deviceBus == BUS_TYPE_STR_SCSI &&
            diskInfo.diskAddress.deviceIndex == m_targetBusIndex) {
            m_attachedDisk = diskInfo;
            m_newCreateVolId = diskInfo.diskAddress.vmdiskUuid;
            if (!SaveTmpVolumeInfoToFile()) {
                ERRLOG("Save tmp volume info to file failed, %s", m_jobId.c_str());
                return false;
            }
            INFOLOG("Get attached snapshot disk");
            return true;
        }
    }
    return false;
}

bool NutanixVolumeHandler::GetNewAttachedDisk()
{
    INFOLOG("enter nutanixvolhandler GetNewAttachedDisk");
    NutanixVMInfo agentVmiAfterAttach;
    if (!GetAgentVmInfo(agentVmiAfterAttach)) {
        ERRLOG("GetAgentVmInfo failed");
        return false;
    }
    if (m_isBackup) {
        if (!FindAttachedDiskByNdfsPath(agentVmiAfterAttach)) {
            ERRLOG("FindAttachedDiskByNdfsPath failed");
            return false;
        }
    } else {
        if (!FindAttachedDiskByTargetBusIndex(agentVmiAfterAttach)) {
            ERRLOG("FindAttachedDiskByTargetBusIndex failed");
            return false;
        }
    }
    if (!GetNewAttachDev(m_attachedDisk)) {
        ERRLOG("Get attached Dev failed, diskid: %s", m_attachedDisk.diskAddress.vmdiskUuid.c_str());
        return false;
    }
    int retryTimes = 0;
    while (retryTimes < SCAN_DISK_RETRY_TIMES) {
        if (boost::filesystem::exists(m_diskDevicePath)) {
            INFOLOG("Device path %s exists.", m_diskDevicePath.c_str());
            return true;
        } else {
            retryTimes++;
            WARNLOG("Attached vol path(%s) is not ready, retry %d times",
                    m_diskDevicePath.c_str(), retryTimes);
            sleep(COMMON_WAIT_TIME);
        }
    }

    ERRLOG("Find the attached vol on agent failed, %s", m_taskId.c_str());
    return false;
}

uint64_t NutanixVolumeHandler::GetAttachedVolNumber()
{
    GetVMInfoRequest req(m_serverId);
    SetCommonInfo(req);
    req.SetVmNicConfig(false);
    if (m_nutanixClient == nullptr) {
        ERRLOG("Get VM info m_nutanixClient nullptr, %s", m_taskId.c_str());
        return 0;
    }
    std::shared_ptr<NutanixResponse<struct NutanixVMInfo>> resp =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct NutanixVMInfo>, GetVMInfoRequest>(req);
    if (resp == nullptr) {
        ERRLOG("Get VM info failed, %s", m_taskId.c_str());
        return 0;
    }
    uint64_t attachedNum = resp->GetResult().vmDiskInfo.size();
    for (auto &disk : resp->GetResult().vmDiskInfo) {
        if (disk.diskAddress.deviceBus != BUS_TYPE_STR_SCSI) {
            --attachedNum;
        }
    }
    return attachedNum;
}

uint64_t NutanixVolumeHandler::GetAttachLimit()
{
    if (g_versionAttachLimit == -1) {
        g_versionAttachLimit = NUTANIX_ATTACH_LIMIT; // SCSI默认256
        INFOLOG("Attach number check: version attach limit %d", g_versionAttachLimit);
    }
    uint64_t attached = GetAttachedVolNumber();
    if (attached == 0) {
        g_attachLimit = m_agentVmInfoBeforeAttach.vmDiskInfo.size();
    } else {
        g_attachLimit = g_versionAttachLimit <= attached ? 0 : g_versionAttachLimit - attached;
    }
    INFOLOG("Attach number check: already attached: %ld, left: %ld, %s", attached, g_attachLimit, m_jobId.c_str());
    return g_attachLimit;
}

int32_t NutanixVolumeHandler::DoAttachVolumeToHost(const std::string &volumeId)
{
    return SUCCESS;
}

bool NutanixVolumeHandler::CheckAndAttachVolume()
{
    INFOLOG("enter nutanixvolhandler CheckAndAttachVolume");
    {
        std::unique_lock<std::mutex> lock(g_attachCntMutex);
        if (GetAttachLimit() == 0) {
            g_attachCntCv.wait(lock, [] {return (g_attachLimit > 0);});
            DBGLOG("Attach number check: can attach %ld scsi disk. %s", g_attachLimit, m_jobId.c_str());
        }
        if (AttachDiskAndGetDev() != SUCCESS) {
            ERRLOG("AttachDiskAndGetDev disk failed. %s", m_jobId.c_str());
            return false;
        }
    }
    return true;
}

bool NutanixVolumeHandler::CheckVolIsAttached(const VmDisk &vol)
{
    return true;
}

bool NutanixVolumeHandler::GetContainerInfo(const std::string &storageContainerId,
    NutanixStorageContainerInfo &storageContainerInfo)
{
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

void NutanixVolumeHandler::SetAttachSnapshot(VmDisk &vol, const VmDisk &m_newCreateVmDisk)
{
    vol.vmDiskClone.snapshotGroupId = m_newCreateVmDisk.vmDiskClone.snapshotGroupId;
    vol.vmDiskClone.storageContainerId = m_newCreateVmDisk.vmDiskClone.storageContainerId;
    std::string res = "/{containerName}/.acropolis/snapshot/{groupId}/vmdisk/{vmdiskId}";
    std::map<std::string, std::string> pathParams = {
        {"containerName", m_storageContainerName},
        {"groupId", vol.vmDiskClone.snapshotGroupId},
        {"vmdiskId", m_newCreateVmDisk.vmDiskClone.diskAddress.vmdiskUuid}
    };
    for (auto &path : pathParams) {
        boost::replace_all(res, "{" + path.first + "}", path.second);
    }
    INFOLOG("path : %s", res.c_str());
    m_ndfsFilePath = res;
    vol.vmDiskClone.diskAddress.ndfsFilepath = m_ndfsFilePath;
}

bool NutanixVolumeHandler::LoadCreatedSnapshotVolumeInfo(const BackupSubJobInfo &subJob,
    VmDisk &newCreateVolClone)
{
    VmDisk tmp;
    if (!Module::JsonHelper::JsonStringToStruct(subJob.m_curSnapshotInfo.m_extendInfo, tmp)) {
        ERRLOG("Load create snapshot vmDiskClone info failed, %s", m_taskId.c_str());
        return false;
    }
    m_storageContainerName = subJob.m_volInfo.m_datastore.m_name;
    tmp.vmDiskClone.snapshotGroupId = subJob.m_curSnapshotInfo.m_snapshotId;
    newCreateVolClone = tmp;
    return true;
}

int32_t NutanixVolumeHandler::Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo)
{
    if (!LoadCreatedSnapshotVolumeInfo(jobInfo, m_newCreateVmDisk)) {
        ERRLOG("Load snapshot info failed.");
        return FAILED;
    }

    if (!CheckAndAttachVolume()) {
        return FAILED;
    }

    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }
    INFOLOG("Start to open disk for backup success.");
    return SUCCESS;
}

int32_t NutanixVolumeHandler::Open(const VolOpenMode &mode)
{
    LOGGUARD("Enter");
    if (!CheckAndAttachVolume()) {
        return FAILED;
    }
    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }

    INFOLOG("Open disk success.");
    return SUCCESS;
}

bool NutanixVolumeHandler::SaveTmpVolumeInfoToFile()
{
    if (m_targetBusIndex < 0) {
        WARNLOG("targetBusIndex is invalid, no need save tmp volume info to file");
        return true;
    }
    NewVolumeInfo newVolInfo;
    std::string strInfo;
    newVolInfo.agentVmUuid = m_serverId;
    newVolInfo.tmpVolUuid = m_newCreateVolId;
    newVolInfo.busIndex = m_targetBusIndex;
    if (!Module::JsonHelper::StructToJsonString(newVolInfo, strInfo)) {
        ERRLOG("StructToJsonString Failed, %s", m_jobId.c_str());
        return false;
    }
    std::string newVolumeInfoDir = m_cacheRepoPath + VIRT_PLUGIN_NEW_VOLUME_INFO;
    if (!m_cacheRepoHandler->Exists(newVolumeInfoDir)) {
        INFOLOG("newVolumeInfoDir(%s) not exist, create it.", newVolumeInfoDir.c_str());
        if (!m_cacheRepoHandler->CreateDirectory(newVolumeInfoDir)) {
            if (!m_cacheRepoHandler->Exists(newVolumeInfoDir)) {
                ERRLOG("Create newVolumeInfoDir failed.(%s). %s", newVolumeInfoDir.c_str(), m_jobId.c_str());
                return false;
            }
        }
    } else {
        INFOLOG("newVolumeInfoDir exist, not create it.");
    }
    std::string newVolumeInfoFile = newVolumeInfoDir + m_volInfo.m_name + ".info";
    if (Utils::SaveToFileWithRetry(m_cacheRepoHandler, newVolumeInfoFile, strInfo) != SUCCESS) {
        ERRLOG("Failed to generate main task status to file, %s", m_jobId.c_str());
        return false;
    }
    INFOLOG("Save tmp volume info to file(%s): %s", newVolumeInfoFile.c_str(), strInfo.c_str());
    return true;
}

int32_t NutanixVolumeHandler::DoDetachVolumeFromeHostInner(const std::string &detachVolId, const std::string &serverId)
{
    // 恢复时，由于nutanix不允许普通vDisk独立于虚拟机存在，卸载前需要克隆并挂载写完的卷到目标位置，延后在后置子任务挂载动作后进行卸载
    std::lock_guard<std::mutex> lock(g_attachVolumeMutex);
    if (!m_isBackup) {
        DBGLOG("Recover mode, skip detach volume ");
        if (!SaveTmpVolumeInfoToFile()) {
            ERRLOG("Save tmp volume info to file failed, %s", m_jobId.c_str());
            return FAILED;
        }
        return SUCCESS;
    }
    DBGLOG("Begin Detach Volume detachVolId : %s", detachVolId.c_str());
    DetachDiskRequest req(serverId, detachVolId);
    SetCommonInfo(req);
    int retryNum {0};
    std::shared_ptr<NutanixResponse<struct TaskMsg>> response =
        m_nutanixClient->ExecuteAPI<NutanixResponse<struct TaskMsg>, DetachDiskRequest>(req);
    if (response == nullptr || !CheckTaskStatus(response->GetResult().taskId)) {
        ERRLOG("Detach disk %s on vm %s failed, %s", detachVolId.c_str(),
            serverId.c_str(), m_taskId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t NutanixVolumeHandler::DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId)
{
    INFOLOG("Enter DoDetachVolumeFromeHost detachVolId : %s", detachVolId.c_str());
    int32_t ret = FAILED;
    if (DoDetachVolumeFromeHostInner(detachVolId, serverId) != SUCCESS) {
        return FAILED;
    }
    std::unique_lock<std::mutex> lock(g_attachCntMutex);
    if (GetAttachLimit() > 0) {
        DBGLOG("Attach number check: notify. %s", m_jobId.c_str());
        g_attachCntCv.notify_one();
    }
    return SUCCESS;
}

int32_t NutanixVolumeHandler::Close()
{
    INFOLOG("Enter NutanixVolumeHandler Close");
    if (m_spDeviceFile->Close() != DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK) {
        std::string errorDes = m_spDeviceFile->GetErrString();
        ERRLOG("Close volume file failed, errorDes:%s", errorDes.c_str());
        return FAILED;
    }

    return CloudVolumeHandler::Close();
}

}