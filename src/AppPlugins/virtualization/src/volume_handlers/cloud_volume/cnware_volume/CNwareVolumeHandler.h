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
#ifndef OPENSTACK_VOLUME_HANDLER_H
#define OPENSTACK_VOLUME_HANDLER_H

#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "log/Log.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "volume_handlers/cloud_volume/CloudVolumeHandler.h"
#include "protect_engines/cnware/api/client/CNwareClient.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/sha256/Sha256.h"
#include "protect_engines/cnware/common/CNwareConstants.h"

using namespace VirtPlugin;

const std::string CNWARE_DISK_STATUS_NORMAL = "normal";

class SnapShotExtenInfo {
public:
    std::string m_groupSnapShotId;      // 快照所属组ID
    std::string m_volumeType;   // 快照原卷所属的VolumeType
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapShotId, group_snapshot_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    END_SERIAL_MEMEBER
};

namespace CNwarePlugin {
extern uint64_t g_attachLimit;
extern std::mutex g_attachCntMutex;
extern std::condition_variable g_attachCntCv;
extern std::mutex g_attachVolumeMutex;
class ProtectExtendInfo {
public:
    std::string m_projectId;
    std::string m_domainName;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    END_SERIAL_MEMEBER
};

class CNwareVolumeHandler : public CloudVolumeHandler {
public:
    CNwareVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId) : CloudVolumeHandler(jobHandle, volInfo, jobId, subJobId)
    {
        m_voluemAvailableStatus = CNWARE_DISK_STATUS_NORMAL;
    }
    virtual ~CNwareVolumeHandler() {};
    bool CheckAndAttachVolume() override;
    bool InitClient();

private:
    int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo) override;
    int32_t Open(const VolOpenMode &mode) override;
    bool FormCreateVolmeInfo(const VolSnapInfo &snapshot, std::string &body);
    int32_t DoAttachVolumeToHost(const std::string &volumeId) override;
    int32_t DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot) override;
    int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 30, uint32_t retryCount = 5) override;
    int32_t DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId) override;
    int32_t DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot);
    bool CheckAndDetachVolume(const std::string &volId) override;
    int32_t Close() override;
    std::string GetProxyHostId() override;

    void SetCommonInfo(CNwareRequest& req);
    bool LoadCreatedSnapshotVolumeInfo(const BackupSubJobInfo &subJob, SnapshotDiskInfo &newCreateVolume);
    bool GetVolInfoOnStorage(const std::string &volId, CNwareDiskInfo &volumeDetail);
    bool CheckTaskStatus(const std::string taskId);
    bool GetAttachedVolInfo(const std::string &volId, const std::vector<std::string> &beforeDevList);
    bool GetNewAttachDev(const std::vector<std::string> &beforeDevList);
    template<typename T>
    bool CommonCheckResponse(const T &response);
    
    bool GetCNwareVersion(std::string &version);
    uint64_t GetAttachLimit();
    uint64_t GetAttachedVolNumber();
    int32_t DoDetachVolumeFromeHostInner(const std::string &detachVolId, const std::string &serverId);
    int32_t DoAttachVolumeToHostInner(const std::string &volId);
    bool CheckVolIsAttached(const std::string &volId);
    bool GetBusDevByVolId(std::string &bus, std::string &dev, const std::string &volId,
        const std::string &serverId);
    bool GetVolBusDev(std::string &busDevStr, const std::string &detachVolId, const std::string &serverId);
    int32_t AttachDiskAndGetDev(const std::string &volId);
    int32_t CheckDetachDevList(const std::vector<std::string> &beforeDevList);
    bool CheckBusDevStr(const std::string &BusDevStr, const std::string &serverId);
    std::string GetHostName();

private:
    std::shared_ptr<CNwareClient> m_cnwareClient = nullptr;
    std::shared_ptr<CertManger> m_certMgr = nullptr;
    SnapshotDiskInfo m_newCreateVolInfo;
    SnapshotInfo m_snapshotInfo;
    std::string m_bus = "";
    std::string m_dev = "";
    std::string m_poolName;
    std::string m_volName;
    std::string m_preallocation = "off"; // 默认值:off 置备类型 off:精简置备 falloc:厚置备延迟置零 full:厚置备置零
    std::string m_serverName = "";
};

}
#endif
