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
#ifndef APPPLUGINS_VIRTUALIZATION_CLOUDVOLUMEHANDLERS_H
#define APPPLUGINS_VIRTUALIZATION_CLOUDVOLUMEHANDLERS_H

#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "log/Log.h"
#include "volume_handlers/VolumeHandler.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "common/sha256/Sha256.h"
#include "protect_engines/openstack/api/cinder/model/VolumeDetail.h"

namespace VirtPlugin {

class NewCreatedVolumeList {
public:
    std::vector<OpenStackPlugin::Volume> m_volumelist;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumelist, volumes)
    END_SERIAL_MEMEBER
};

class CloudVolumeHandler : public VolumeHandler {
public:
    CloudVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId) : VolumeHandler(jobHandle, volInfo, jobId, subJobId)
    { ifmarkBlockValidData = false; }
    virtual ~CloudVolumeHandler() {};
    virtual bool CheckAndAttachVolume() = 0;
    virtual int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo);
    int32_t Open(const VolOpenMode &mode);
    int32_t InitializeVolumeInfo(const std::string &confFile);
    int32_t GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
                           DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset);
    int32_t ReadBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, std::shared_ptr<uint8_t[]> &buffer,
        std::shared_ptr<uint8_t[]> &calBuffer, std::shared_ptr<uint8_t[]> &readBuffer);
    int32_t WriteBlocks(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, std::shared_ptr<uint8_t[]> &buffer);
    int32_t Close();
    uint64_t GetVolumeSize();
    int32_t TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode);
    int32_t CleanLeftovers();
    int32_t Flush();
    int32_t CalculateHashValue(const std::shared_ptr<unsigned char[]> &buffer,
        const std::shared_ptr<uint8_t[]> &calcHashBuffer, std::shared_ptr<uint8_t[]> &shaBuffer,
        uint64_t bufferSize) override;
    int32_t NeedToBackupBlock(std::shared_ptr<uint8_t[]> &buffer, struct io_event* event) override;

protected:
    template<typename T>
    void SetRequest(T &request, bool useAdmin = false);

protected:
    virtual int32_t DoAttachVolumeToHost(const std::string &volumeId) = 0;
    virtual int32_t DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot) = 0;
    virtual int32_t DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId) = 0;
    virtual bool CheckAndDetachVolume(const std::string &volId) = 0;
    virtual std::string GetProxyHostId() = 0;
    int32_t DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot);
    bool LoadCreatedVolumeInfoFromeFile(const VolSnapInfo &snapshot, OpenStackPlugin::Volume &newCreateVolume);
    void ExpandWaitTime(uint32_t curRetryTime, uint32_t &totalTimes, const std::string &state,
        std::vector<std::string> intermediateState);
    int32_t DoGetServerVolume();
    int32_t GetVolumePath(const std::string &volumeId, std::map<std::string, std::set<std::string>> &diskPathMap);
    int DoScanDisk(const std::string &volumeId, int32_t retryTimes);
    int32_t GetDiskPathList(std::map<std::string, std::set<std::string>> &diskPathMap);
    bool ChangeFilePriviledge(const std::string &file, const VolOpenMode &mode);
    int32_t DetachVolumeHandle(const std::string &volId, const std::string &serverId);
    int32_t ListDev(std::vector<std::string> &devList);
    int32_t UpdateDiskDev();
    int32_t GetIscsiDiskPath(const std::string &volumeId);
    std::string GenNewVolumeName(const std::string &snapshotId)
    {
        return "Backup_volume_" + snapshotId + "_" + std::to_string(time(nullptr));
    }

    std::string GetNewVolumeDesciption()
    {
        return "Backup clone volume created by host " + m_serverId + ", task id " + m_taskId;
    }

    std::shared_ptr<DiskDeviceFile> GetDiskDeviceFile() override
    {
        return m_spDeviceFile;
    }

protected:
    bool ReadHashData(uint64_t startAddr, std::shared_ptr<uint8_t[]> &hashBuffer);
    int32_t BackupInitBaseInfo();
    bool UpdateHashValue(uint64_t startAddr, std::shared_ptr<uint8_t[]> &hashBuffer);

    static std::shared_ptr<uint8_t[]> GetAllZeroDirtyRangeShaPtr();

protected:
    bool m_isBackup = false;
    std::string m_newCreateVolId;
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile = nullptr;
    uint64_t m_volSizeInBytes {0};
    VolumeDSExtendInfo m_dsExtend;
    std::string m_serverId = "";
    std::string m_diskDevicePath;
    std::string m_snapshotCreateVolumeFile;
    AppProtect::BackupJobType m_backupType;
    std::string m_volHashFile;
    bool m_volHashFileIsExist { false };
    uint64_t m_hashFileSize {0};
    AppProtect::ApplicationEnvironment m_appEnv;
    AppProtect::Application m_application;
    std::string m_taskId;
    std::string m_voluemAvailableStatus;
    std::string m_volumeInUseStatus;
    static std::shared_ptr<uint8_t[]> m_allZeroDirtyRangeShaPtr;
    OpenStackPlugin::Volume m_newCreateVolInfo;
    NewCreatedVolumeList m_newCreatedVolumeList;  // 本次备份所有快照创建的卷信息
    static std::mutex m_attachVolumeMutex;
    static std::mutex m_fileMutex;
};
}
#endif // APPPLUGINS_VIRTUALIZATION_CLOUDVOLUMEHANDLERS_H