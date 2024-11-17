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
#ifndef NUTANIX_VOLUME_HANDLER_H
#define NUTANIX_VOLUME_HANDLER_H

#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "log/Log.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "volume_handlers/cloud_volume/CloudVolumeHandler.h"
#include "protect_engines/nutanix/common/ErrorCode.h"
#include "protect_engines/nutanix/api/client/NutanixClient.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/sha256/Sha256.h"
#include "protect_engines/nutanix/common/NutanixConstants.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"
#include "protect_engines/nutanix/api/client/model/QueryTaskRequest.h"

using namespace VirtPlugin;

namespace NutanixPlugin {
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

class NutanixVolumeHandler : public CloudVolumeHandler {
public:
    NutanixVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId) : CloudVolumeHandler(jobHandle, volInfo, jobId, subJobId)
    {
    }
    virtual ~NutanixVolumeHandler() {};
    bool CheckAndAttachVolume() override;
    bool InitClient();

private:
    int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo) override;
    int32_t Open(const VolOpenMode &mode) override;
    int32_t DoAttachVolumeToHost(const std::string &volumeId) override;
    int32_t DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot) override;
    int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 30, uint32_t retryCount = 5) override;
    int32_t DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId) override;
    int32_t DoCreateVolumeFromSnapShotId(const VolSnapInfo &snapshot);
    bool CheckAndDetachVolume(const std::string &volId) override;
    int32_t Close() override;
    std::string GetProxyHostId() override;

    void SetCommonInfo(NutanixRequest& req);
    bool CheckTaskStatus(const std::string taskId);
    int32_t DoAttachVolumeToHost(const VmDisk &vol);
    bool GetContainerInfo(const std::string &storageContainerId, NutanixStorageContainerInfo &storageContainerInfo);
    bool LoadCreatedSnapshotVolumeInfo(const BackupSubJobInfo &subJob, VmDisk &newCreateVolClone);
    bool GetAttachedDisk(const std::string &vmId, const VmDisk &vol, VmDiskInfo &attachedDisk);
    bool DoAttachDisk(const VmDisk &vol);
    int32_t DetachVolumeHandle(const VmDiskInfo &detachVol, const std::string &serverId);
    int32_t DoDetachVolumeFromeHostInner(const std::string &detachVolId, const std::string &serverId);
    void SetAttachSnapshot(VmDisk &vol, const VmDisk &m_newCreateVmDisk);
    uint64_t GetAttachLimit();
    uint64_t GetAttachedVolNumber();
    bool GetNewAttachedDisk();
    bool GetNewAttachDev(VmDiskInfo diskInfo);
    int32_t AttachDiskAndGetDev();
    int32_t DoAttachVolumeToHostInner();
    bool CheckVolIsAttached(const VmDisk &vol);
    // Restore
    bool GetTargetBusIndex();
    void SetNewCreateVmDiskInfo(AttachCreateNewVmDisk &newDisk);
    bool GetAgentVmInfo(NutanixVMInfo &vmInfo);
    bool FindAttachedDiskByNdfsPath(const NutanixVMInfo &agentVmiAfterAttach);
    bool FindAttachedDiskByTargetBusIndex(const NutanixVMInfo &agentVmiAfterAttach);
    bool SaveTmpVolumeInfoToFile();

private:
    std::shared_ptr<NutanixClient> m_nutanixClient = nullptr;
    std::shared_ptr<CertManger> m_certMgr = nullptr;
    VmDisk m_newCreateVmDisk;
    VmDiskInfo m_attachedDisk;
    SnapshotInfo m_snapshotInfo;
    std::string m_storageContainerName;
    std::string m_bus = "";
    std::string m_dev = "";
    std::string m_poolName;
    std::string m_volName;
    std::string m_serverName = "";
    std::string m_ndfsFilePath = "";
    NutanixVMInfo m_agentVmInfoBeforeAttach;
    int32_t m_targetBusIndex = -1;
};

}
#endif