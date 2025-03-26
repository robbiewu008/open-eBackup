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
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/nova/NovaClient.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/sha256/Sha256.h"

using namespace VirtPlugin;

const std::string OPENSTAK_VOLUME_AVAILABLE_STATUS = "available";
const std::string OPENSTACK_VOLUME_IN_USE_STATUS = "in-use";
const std::string OPENSTACK_VOLUME_DETACHING_STATUS = "detaching";

class SnapShotExtenInfo {
public:
    std::string m_groupSnapShotId;      // 快照所属组ID
    std::string m_volumeType;   // 快照原卷所属的VolumeType
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapShotId, group_snapshot_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    END_SERIAL_MEMEBER
};

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class ProtectExtendInfo {
public:
    std::string m_projectId;
    std::string m_domainName;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, projectId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domainName, domainName)
    END_SERIAL_MEMEBER
};

class CinderVolumeHandler : public CloudVolumeHandler {
public:
    CinderVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId) : CloudVolumeHandler(jobHandle, volInfo, jobId, subJobId)
    {
        m_voluemAvailableStatus = OPENSTAK_VOLUME_AVAILABLE_STATUS;
        m_volumeInUseStatus = OPENSTACK_VOLUME_IN_USE_STATUS;
    }
    virtual ~CinderVolumeHandler() {};
    bool CheckAndAttachVolume() override;

private:
    template<typename T>
    void SetOpenStackRequest(T &request, bool useAdmin = false);

private:
    int32_t DoAttachVolumeToHost(const std::string &volumeId) override;
    int32_t DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot) override;
    int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 30, uint32_t retryCount = 5) override;
    int32_t DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId) override;
    bool CheckAndDetachVolume(const std::string &volId) override;
    std::string GetProxyHostId() override;

private:
    bool WaitVolumeStatus(const Volume &volDetail, std::string &status);
    bool GetVolumeDetail(const std::string &volId, Volume &volDetail);
    bool UpdateNewVolumeInfoToFile(Volume &newCreateVolume);
    bool FormCreateVolmeInfo(const VolSnapInfo &snapshot, std::string &body);
    bool CheckCreateFullCloneVolum();

private:
    OpenStackPlugin::NovaClient m_novaClient;
    OpenStackPlugin::CinderClient m_cinderClient;
    std::string m_domainName = "";
};

OPENSTACK_PLUGIN_NAMESPACE_END
#endif // OPENSTACK_VOLUME_HANDLER_H
