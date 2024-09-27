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
#include "CloudVolumeHandler.h"
#include <boost/uuid/uuid_io.hpp>
#include "common/Structs.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/DiskCommDef.h"

namespace {
const std::string MODULE_NAME = "CloudVolumeHandler";
const std::string D_FINDDISK = "../";
const int32_t WWN_POS_OFFSET = 7;
const std::string CLOUD_VOLUME_CONF = "CloudVolumeConfig";
const std::string CLOUD_VOLUME_AVAILABLE_STATUS = "available";
const std::string CLOUD_VOLUME_IN_USE_STATUS = "in-use";
const std::string CLOUD_VOLUME_DETACHING_STATUS = "detaching";
const uint32_t GB = 1024 * 1024 * 1024UL;
const std::int32_t SLEEP_TIMES = 30;
constexpr uint32_t RETRY_INTERVAL_SECOND = 5;
const uint32_t MAX_EXEC_COUNT = 5;
const uint8_t BIT_WITH = 8;
const uint8_t DEFAULT_RETRY_INTEVAL = 60;
const uint32_t QUERY_CREATE_VOL_RETRY_INTERVAL = 240;
const uint32_t PER_GB_RETYR_TIMES = 5;
const uint32_t RETRY_COUNT = 10;
const uint32_t MAX_RETRY_TIMES = 20;
// 	错误场景：执行调用生产端接口操作时，由于调用生产端接口失败，操作失败。
constexpr int64_t INVOKE_API_FAILED_GENERAL_CODE = 1577213460;
}

std::shared_ptr<uint8_t[]> VirtPlugin::VolumeHandler::m_allZeroDirtyRangeDataPtr = nullptr;
std::shared_ptr<uint8_t[]> VirtPlugin::CloudVolumeHandler::m_allZeroDirtyRangeShaPtr = nullptr;
namespace VirtPlugin {
std::mutex CloudVolumeHandler::m_attachVolumeMutex;
std::mutex CloudVolumeHandler::m_fileMutex;

int32_t CloudVolumeHandler::NeedToBackupBlock(std::shared_ptr<uint8_t[]> &buffer, struct io_event* event)
{
    if (memcmp(buffer.get(), GetAllZeroDirtyRangeDataPtr().get(), DIRTY_RANGE_BLOCK_SIZE) == 0) {
        return DATA_ALL_ZERO_IGNORE_WRITE;
    }
    return CALCULATE_INITIAL_STATE;
}

int32_t CloudVolumeHandler::InitializeVolumeInfo(const std::string &confOption)
{
    if (m_jobHandle == nullptr) {
        ERRLOG("Job handle is null.");
        return FAILED;
    }
    m_isBackup = m_jobHandle->GetJobType() == JobType::BACKUP;
    m_volSizeInBytes = m_volInfo.m_volSizeInBytes;  // 卷大小
    m_spDeviceFile = std::make_shared<DiskDeviceFile>();
    m_taskId = m_jobHandle->GetTaskId();
    if (m_spDeviceFile.get() == nullptr) {
        ERRLOG("Create DiskDeviceFile failed.");
        return FAILED;
    }
    InitRepoHandler();
    if (m_isBackup && BackupInitBaseInfo() != SUCCESS) {
        return FAILED;
    }
    m_serverId = GetProxyHostId();
    if (m_serverId.empty()) {
        ERRLOG("Get server id failed, %s.", m_taskId.c_str());
        return FAILED;
    }

    m_appEnv = m_jobHandle->GetAppEnv();
    m_application = m_jobHandle->GetApp();
    return SUCCESS;
}

int32_t CloudVolumeHandler::BackupInitBaseInfo()
{
    if (m_metaRepoHandler == nullptr || m_volumeInfoRepoHandler == nullptr) {
        ERRLOG("Create meta repository failed.");
        return FAILED;
    }
    m_volHashFile = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_HASH_DIR + m_volInfo.m_uuid + "_hash.record";
    m_backupType = m_jobHandle->GetBackupType();
    m_volHashFileIsExist = m_metaRepoHandler->Exists(m_volHashFile);
    m_snapshotCreateVolumeFile = m_metaRepoPath + VIRT_PLUGIN_SNAPHOT_CREATE_VOLUME_INFO;
    if (m_backupType == AppProtect::BackupJobType::FULL_BACKUP || (!m_volHashFileIsExist)) {
        m_backupType = AppProtect::BackupJobType::FULL_BACKUP;
        std::string hashDir = m_metaRepoPath + VIRT_PLUGIN_VOLUMES_HASH_DIR;
        m_metaRepoHandler->CreateDirectory(hashDir);
        if (m_metaRepoHandler->Open(m_volHashFile, "w") != SUCCESS) {
            ERRLOG("Create file %s failed.", m_volHashFile.c_str());
            return FAILED;
        }
        if (m_metaRepoHandler->Close() != SUCCESS) {
            ERRLOG("Close file %s failed.", m_volHashFile.c_str());
            return FAILED;
        }
    }

    if (m_metaRepoHandler->Open(m_volHashFile, "r+") != SUCCESS) {
        ERRLOG("Open file(%s) failed.", m_volHashFile.c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t CloudVolumeHandler::GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
    DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset)
{
    dirtyRanges.UseFull();
    DirtyRange everything(startOffset, endOffset - startOffset);
    (void) dirtyRanges.AddRange(everything);
    return SUCCESS;
}

template<typename T>
void CloudVolumeHandler::SetRequest(T &request, bool useAdmin)
{
    AuthObj auth;
    if (useAdmin) {
        auth.name = m_appEnv.auth.authkey;
        auth.passwd = m_appEnv.auth.authPwd;
    } else {
        auth.name = m_application.auth.authkey;
        auth.passwd = m_application.auth.authPwd;
    }
    request.SetUserInfo(auth);
    request.SetEnvAddress(m_appEnv.endpoint);
    return;
}

int32_t CloudVolumeHandler::DoAttachVolumeToHost(const std::string &volumeId)
{
    return SUCCESS;
}

int32_t CloudVolumeHandler::DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    if (LoadCreatedVolumeInfoFromeFile(snapshot, m_newCreateVolInfo)) {
        INFOLOG("snapshot(%s) has been created volume(%s).", m_newCreateVolInfo.m_snapshotId.c_str(),
            m_newCreateVolInfo.m_id.c_str());
        return SUCCESS;
    }
    if (DoCreateNewVolumeFromSnapShotId(snapshot) != SUCCESS) {
        ERRLOG("Snapshot(%s) create volume failed.", snapshot.m_snapshotId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool CloudVolumeHandler::LoadCreatedVolumeInfoFromeFile(const VolSnapInfo &snapshot,
    OpenStackPlugin::Volume &newCreateVolume)
{
    if (m_volumeInfoRepoHandler->Exists(m_snapshotCreateVolumeFile)) {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        if (Utils::LoadFileToStructWithRetry(m_volumeInfoRepoHandler, m_snapshotCreateVolumeFile,
            m_newCreatedVolumeList) != SUCCESS) {
            ERRLOG("Load created volume list info failed.");
            return false;
        }

        for (auto volume : m_newCreatedVolumeList.m_volumelist) {
            if (volume.m_snapshotId == snapshot.m_snapshotId && !volume.m_id.empty()) {
                INFOLOG("snapshot(%s) has been created volume(%s).", volume.m_snapshotId.c_str(), volume.m_id.c_str());
                newCreateVolume = volume;
                m_newCreateVolId = volume.m_id;
                return true;
            }
        }
    }
    ERRLOG("Not found snapshotCreateVolumeFile id director.");
    return false;
}

int32_t CloudVolumeHandler::DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot)
{
    return SUCCESS;
}

/**
 * @brief 根据卷当前状态判断是否需要继续延长等待时间
 *
 */
void CloudVolumeHandler::ExpandWaitTime(uint32_t curRetryTime, uint32_t &totalTimes, const std::string &state,
    std::vector<std::string> intermediateState)
{
    if (curRetryTime != totalTimes || totalTimes > MAX_RETRY_TIMES) {
        return;
    }
    for (const auto &interState: intermediateState) {
        if (interState == state) {
            totalTimes += RETRY_COUNT;
            return;
        }
    }
}

int32_t CloudVolumeHandler::ListDev(std::vector<std::string> &devList)
{
    std::string diskPath;
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::vector<Module::CmdParam> cmdParam{
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "scan_dev"
    };
    if (Module::RunCommand("sudo", cmdParam, cmdOut) != 0) {
        ERRLOG("lsblk command is not enable.");
        return FAILED;
    }
    for (const std::string& buff : cmdOut) {
        std::string strLine(buff);
        size_t pos = strLine.find('\n');
        if (pos != std::string::npos) {
            strLine.erase(pos, 1);
        }
        if (strLine.empty()) {
            ERRLOG("Do not find a dev!");
            return FAILED;
        }
        devList.emplace_back(strLine);
    }
    return SUCCESS;
}

int32_t CloudVolumeHandler::UpdateDiskDev()
{
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::vector<Module::CmdParam> cmdParam{
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "update_disk_dev"
    };
    if (Module::RunCommand("sudo", cmdParam, cmdOut) != 0) {
        ERRLOG("Exec command partprobe failed.");
        return FAILED;
    }
    DBGLOG("Update disk dev success.");
    return SUCCESS;
}

int32_t CloudVolumeHandler::GetDiskPathList(std::map<std::string, std::set<std::string>> &diskPathMap)
{
    std::string diskPath;
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::vector<Module::CmdParam> cmdParam{
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "get_virtio_diskpath_list"
    };
    if (Module::RunCommand("sudo", cmdParam, cmdOut) != 0) {
        ERRLOG("ls -l /dev/disk/by-id/virtio-* command is not enable.");
        return FAILED;
    }

    for (const std::string& buff : cmdOut) {  // 逐行读取，获取盘符
        std::string strLine(buff);
        std::string strLineCopy(buff);
        int nPos = strLine.find(D_FINDDISK);
        if (nPos == std::string::npos) {
            continue;
        }

        strLine.erase(strLine.find_last_not_of("\n") + 1); // 去除分区干扰
        INFOLOG("Line:%s", strLine.c_str());
        strLine.erase(0, nPos);
        strLine = "/dev/disk/by-id/" + strLine;
        int nPosWWN = strLineCopy.find("virtio-");
        if (nPosWWN == -1) {
            continue;
        }
        int offset = WWN_POS_OFFSET;
        strLineCopy.erase(0, nPosWWN);
        int nPosSpace = strLineCopy.find(" ");
        strLineCopy.erase(nPosSpace);
        strLineCopy.erase(0, offset);
        std::map<std::string, std::set<std::string>>::iterator diskMapIt = diskPathMap.find(strLine);
        std::set<std::string> pathSet;
        if (diskMapIt != diskPathMap.end()) {
            pathSet = diskMapIt->second;
        }
        pathSet.insert(strLineCopy);
        diskPathMap[strLine] = pathSet;
    }
    return SUCCESS;
}

int32_t CloudVolumeHandler::GetVolumePath(const std::string &volumeId,
    std::map<std::string, std::set<std::string>> &diskPathMap)
{
    if (diskPathMap.empty()) {
        ERRLOG("Get disk path is empty.");
        return FAILED;
    }
    std::string diskId = Module::ConfigReader::getString("ApsaraStackConfig", "FakeDiskId");
    std::string tmpId = diskId == "Default" ? volumeId : diskId;
    std::map<std::string, std::set<std::string>>::iterator it;
    for (it = diskPathMap.begin(); it != diskPathMap.end(); ++it) {
        std::set<std::string>::iterator pathSetIt = it->second.begin();
        if (tmpId.find(*pathSetIt) != string::npos) {
            m_diskDevicePath = it->first;
            DBGLOG("Get device path is %s.", m_diskDevicePath.c_str());
            return SUCCESS;
        }
    }
    ERRLOG("Not find disk path, uuid %s.", volumeId.c_str());
    return FAILED;
}

int CloudVolumeHandler::DoScanDisk(const std::string &volumeId, int32_t retryTimes)
{
    while (retryTimes > 0) {
        std::map<std::string, std::set<std::string>> diskPathMap;
        diskPathMap.clear();
        UpdateDiskDev();
        int32_t ret = GetDiskPathList(diskPathMap);
        if (ret == SUCCESS && GetVolumePath(volumeId, diskPathMap) == SUCCESS) {
            return SUCCESS;
        }
        retryTimes--;
        if (retryTimes > 0) {
            WARNLOG("Retry to scan disk, volId:%s", volumeId.c_str());
            sleep(SLEEP_TIMES);
        }
    }
    return FAILED;
}

int32_t CloudVolumeHandler::DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId)
{
    return SUCCESS;
}

int32_t CloudVolumeHandler::Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo)
{
    VolSnapInfo snapshot = jobInfo.m_curSnapshotInfo;
    // 将快照信息创建成卷
    if (DoCreateVolumeFromSnapShotId(snapshot) != SUCCESS) {
        ERRLOG("Create Volume from snapshot failed.");
        return FAILED;
    }

    if (!CheckAndAttachVolume()) {
        return FAILED;
    }

    if (DoScanDisk(m_newCreateVolId, MAX_EXEC_COUNT) != SUCCESS) {
        ERRLOG("Scan disk failed.");
        return FAILED;
    }

    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }

    INFOLOG("Start to open disk for backup success.");
    return SUCCESS;
}

bool CloudVolumeHandler::ChangeFilePriviledge(const std::string &file, const VolOpenMode &mode)
{
    std::vector<std::string> paramList;
    std::string permissions = mode == VolOpenMode::READ_WRITE ? "o+w" : "o+r";
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::vector<std::string> cmdOut;
    std::unordered_set<std::string> pathWhitelist;
    pathWhitelist.insert(file);
    std::vector<Module::CmdParam> cmdParam{
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        "change_priviledge",
        Module::CmdParam(Module::COMMON_PARAM, permissions),
        Module::CmdParam(Module::PATH_PARAM, file)
    };
    if (Module::RunCommand("sudo", cmdParam, cmdOut, pathWhitelist) != 0) {
        ERRLOG("Add o+r priviledge to %s failed.", file.c_str());
        return false;
    }
    return true;
}

bool CloudVolumeHandler::CheckAndDetachVolume(const std::string &volId)
{
    return true;
}

bool CloudVolumeHandler::CheckAndAttachVolume()
{
    return true;
}

int32_t CloudVolumeHandler::Open(const VolOpenMode &mode)
{
    int32_t retryCounts = 1;
    // 若执行子任务时虚拟化插件进程重拉，此时不需要再进行挂载卷
    if (DoScanDisk(m_volInfo.m_uuid, retryCounts) != SUCCESS) {
        DBGLOG("Scan disk failed for volume(%s), attach it.", m_volInfo.m_uuid.c_str());
        // 多代理恢复, 异常场景下卷有可能已经挂载到其他代理上，需要先卸载
        if (!CheckAndDetachVolume(m_volInfo.m_uuid)) {
            ERRLOG("Check and detach volume(%s) failed.", m_volInfo.m_uuid.c_str());
            return FAILED;
        }
        // 将卷挂载到主机
        for (int i = 0; i < RETRY_COUNT + 1; i++) {
            if (DoAttachVolumeToHost(m_volInfo.m_uuid) == SUCCESS) {
                INFOLOG("Attach Volume(%s) to host success.", m_volInfo.m_uuid.c_str());
                break;
            }
            WARNLOG("Attach Volume(%s) to host(%s) failed.", m_volInfo.m_uuid.c_str(), m_serverId.c_str());
            if (i == RETRY_COUNT) {
                m_reportArgs = { m_volInfo.m_uuid, m_serverId};
                m_reportPara = {
                    "virtual_plugin_cinder_volume_attach_volume_failed_label",
                    JobLogLevel::TASK_LOG_ERROR,
                    SubJobStatus::FAILED, 0, 0 };
                return FAILED;
            }
        }
        if (DoScanDisk(m_volInfo.m_uuid, MAX_EXEC_COUNT) != SUCCESS) {
            ERRLOG("Scan disk failed for volume(%s).", m_volInfo.m_uuid.c_str());
            return FAILED;
        }
    }
    if (!ChangeFilePriviledge(m_diskDevicePath, mode)) {
        return FAILED;
    }
    int32_t owMode = (mode == VolOpenMode::READ_WRITE) ? O_WRONLY : O_RDONLY;
    DISK_DEVICE_RETURN_CODE deviceCode = m_spDeviceFile->Open(m_diskDevicePath, owMode, m_volSizeInBytes);
    if (deviceCode != DISK_DEVICE_OK) {
        ERRLOG("Open disk device file[%s] failed.", m_diskDevicePath.c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool CloudVolumeHandler::ReadHashData(uint64_t startAddr, std::shared_ptr<uint8_t[]> &hashBuffer)
{
    int32_t retryTimes = 0;
    while (retryTimes < MAX_EXEC_COUNT) {
        if (m_metaRepoHandler->Seek(startAddr) != SUCCESS) {
            ERRLOG("Seek hash file <%llu> failed", startAddr);
            sleep(RETRY_INTERVAL_SECOND);
            retryTimes++;
            continue;
        }
        if (m_metaRepoHandler->Read(hashBuffer, SHA256_DIGEST_LENGTH) == SHA256_DIGEST_LENGTH) {
            return true;
        }
        ERRLOG("Read hash to file failed, try again.");
        sleep(RETRY_INTERVAL_SECOND);
        retryTimes++;
    }
    return false;
}

/**
 * @brief 1. 读取4M块数据  2. 校验4M块数据是否有变化，无变化则继续读下一个4M块，有变化则返回
 *
 * @param offsetInBytes
 * @param bufferSizeInBytes
 * @param buffer
 * @return int32_t
 */
int32_t CloudVolumeHandler::ReadBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer, std::shared_ptr<uint8_t[]> &calBuffer, std::shared_ptr<uint8_t[]> &readBuffer)
{
    if (m_spDeviceFile->Read(offsetInBytes, bufferSizeInBytes, buffer) != DISK_DEVICE_OK) {
        ERRLOG("Read volume block failed, err desc:%s", m_spDeviceFile->GetErrString().c_str());
        return FAILED;
    }
    uint64_t readStartAddr = offsetInBytes / DEFAULT_BLOCK_SIZE * SHA256_DIGEST_LENGTH;

    if (m_backupType == AppProtect::BackupJobType::INCREMENT_BACKUP) {
        if (CalculateSha256::CalculateSha256Value(buffer, bufferSizeInBytes, calBuffer) != SUCCESS) {
            ERRLOG("Volume calculate hash data failed. start addr: %lld", offsetInBytes);
            return FAILED;
        }
        if (ReadHashData(readStartAddr, readBuffer)) {
            if (memcmp(readBuffer.get(), calBuffer.get(), SHA256_DIGEST_LENGTH) == 0) {
                return DATA_SAME_IGNORE_WRITE;
            }
        } else {
            WARNLOG("Read volume hash data failed, start addr: %llu, change the backuptype to full backup",
                readStartAddr);
            m_backupType = AppProtect::BackupJobType::FULL_BACKUP;
        }

        if (!UpdateHashValue(readStartAddr, calBuffer)) {
            ERRLOG("Failed to update hash value, start addr(%lld).", readStartAddr);
            return FAILED;
        }
    } else if (m_backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        if (memcmp(buffer.get(), GetAllZeroDirtyRangeDataPtr().get(), DIRTY_RANGE_BLOCK_SIZE) == 0) {
            calBuffer = GetAllZeroDirtyRangeShaPtr();
            if (calBuffer == nullptr) {
                ERRLOG("Failed to get all zero dirty range sha ptr.");
                return FAILED;
            }
            if (!UpdateHashValue(readStartAddr, calBuffer)) {
                ERRLOG("Failed to update hash value, start addr(%lld).", readStartAddr);
                return FAILED;
            }
            return DATA_ALL_ZERO_IGNORE_WRITE;
        }
        if (CalculateSha256::CalculateSha256Value(buffer, bufferSizeInBytes, calBuffer) != SUCCESS) {
            ERRLOG("Volume calculate hash data failed. start addr: %lld", offsetInBytes);
            return FAILED;
        }
        if (!UpdateHashValue(readStartAddr, calBuffer)) {
            ERRLOG("Failed to update hash value, start addr(%lld).", readStartAddr);
            return FAILED;
        }
    }
    return SUCCESS;
}

std::shared_ptr<uint8_t[]> CloudVolumeHandler::GetAllZeroDirtyRangeShaPtr()
{
    if (m_allZeroDirtyRangeShaPtr != nullptr) {
        return m_allZeroDirtyRangeShaPtr;
    }
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (m_allZeroDirtyRangeShaPtr == nullptr) {
        std::shared_ptr<unsigned char[]> dirtyRangeDataPtr = std::make_unique<unsigned char[]>(DIRTY_RANGE_BLOCK_SIZE);
        memset_s(dirtyRangeDataPtr.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
        m_allZeroDirtyRangeShaPtr = std::make_unique<unsigned char[]>(SHA256_DIGEST_LENGTH);
        if (CalculateSha256::CalculateSha256Value(dirtyRangeDataPtr, static_cast<uint64_t>(DIRTY_RANGE_BLOCK_SIZE),
                                                  m_allZeroDirtyRangeShaPtr) != SUCCESS) {
            ERRLOG("Cal all zero data's sha256 value failed.");
        }
    }
    return m_allZeroDirtyRangeShaPtr;
}

bool CloudVolumeHandler::UpdateHashValue(uint64_t startAddr, std::shared_ptr<uint8_t[]> &hashBuffer)
{
    int32_t retryTimes = 0;
    while (retryTimes < MAX_EXEC_COUNT) {
        {
            std::lock_guard<std::mutex> lock(m_metaRepoHandler->m_repoMutex);
            if (m_metaRepoHandler->Seek(startAddr) != SUCCESS) {
                ERRLOG("Seek hash file <%llu> failed", startAddr);
                sleep(RETRY_INTERVAL_SECOND);
                retryTimes++;
                continue;
            }
            if (m_metaRepoHandler->Write(hashBuffer, SHA256_DIGEST_LENGTH) == SHA256_DIGEST_LENGTH) {
                return true;
            }
        }
        ERRLOG("Write hash to file failed, try again.");
        sleep(RETRY_INTERVAL_SECOND);
        retryTimes++;
    }
    return false;
}

int32_t CloudVolumeHandler::WriteBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer)
{
    if (m_spDeviceFile.get() == nullptr || buffer == nullptr) {
        ERRLOG("Disk device file is empty.");
        return Module::FAILED;
    }
    if (m_spDeviceFile->Write(offsetInBytes, bufferSizeInBytes, buffer) != DISK_DEVICE_OK) {
        ERRLOG("Write volume block failed, err desc:%s", m_spDeviceFile->GetErrString().c_str());
        return Module::FAILED;
    }
    int32_t ret = SUCCESS;
    TP_START("TP_WriteBlocks", 1, &ret);
    TP_END
    if (ret == FAILED) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int32_t CloudVolumeHandler::DetachVolumeHandle(const std::string &volId, const std::string &serverId)
{
    for (int i = 0; i < RETRY_COUNT; i++) {
        if (DoDetachVolumeFromeHost(volId, serverId) == SUCCESS) {
            INFOLOG("Detach volume(%s) from host(%s) success.", volId.c_str(), serverId.c_str());
            return SUCCESS;
        }
        WARNLOG("Detach volume(%s) frome host(%s) failed.", volId.c_str(), serverId.c_str());
    }
    return FAILED;
}

int32_t CloudVolumeHandler::Close()
{
    std::string detachVolId = m_isBackup ? m_newCreateVolId : m_volInfo.m_uuid;
    // 将卷从主机上卸载
    if (DetachVolumeHandle(detachVolId, m_serverId) != SUCCESS) {
        ERRLOG("Detach volume(%s) frome host(%s) failed.", detachVolId.c_str(), m_serverId.c_str());
        return FAILED;
    }
    // 查询卷状态
    if (DoWaitVolumeStatus(detachVolId, m_voluemAvailableStatus) != SUCCESS) {
        ERRLOG("Volume(%s) Status is not available.", detachVolId.c_str());
        return FAILED;
    }

    if (m_metaRepoHandler != nullptr && m_metaRepoHandler->Close() != SUCCESS) {
        ERRLOG("Close meta repo handler failed.");
        return FAILED;
    }
    INFOLOG("Close Volume(%s) handler success.", m_volInfo.m_uuid.c_str());
    return SUCCESS;
}

uint64_t CloudVolumeHandler::GetVolumeSize()
{
    return m_volSizeInBytes;
}

int32_t CloudVolumeHandler::TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode)
{
    return SUCCESS;
}

int32_t CloudVolumeHandler::CleanLeftovers()
{
    return SUCCESS;
}

int32_t CloudVolumeHandler::Flush()
{
    return m_spDeviceFile->Flush() == DISK_DEVICE_OK ? Module::SUCCESS : Module::FAILED;
}

int32_t CloudVolumeHandler::CalculateHashValue(const std::shared_ptr<unsigned char[]> &buffer,
    const std::shared_ptr<uint8_t[]> &calcHashBuffer, std::shared_ptr<uint8_t[]> &shaBuffer, uint64_t bufferSize)
{
    memcpy_s(shaBuffer.get(), SHA256_DIGEST_LENGTH, calcHashBuffer.get(), SHA256_DIGEST_LENGTH);
    return SUCCESS;
}
}