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
#ifndef APSARA_VOLUME_HANDLER_H
#define APSARA_VOLUME_HANDLER_H

#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "log/Log.h"
#include "common/execute_python/ExecutePython.h"
#include "volume_handlers/common/DiskDeviceFile.h"
#include "volume_handlers/common/ControlDevice.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "volume_handlers/cloud_volume/CloudVolumeHandler.h"
#include "volume_handlers/cloud_volume/cinder_volume/CinderVolumeHandler.h"
#include "protect_engines/apsara_stack/common/Structs.h"

using namespace OpenStackPlugin;
const std::string APSARA_DISK_AVAILABLE_STATUS = "Available";
const std::string APSARA_DISK_IN_USE_STATUS = "In_use";
const std::string APSARA_DISK_DETACHING_STATUS = "Detaching";

namespace ApsaraStackPlugin {

class ApsaraVolumeHandler : public CloudVolumeHandler {
public:
    ApsaraVolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId) : CloudVolumeHandler(jobHandle, volInfo, jobId, subJobId)
    {
        m_voluemAvailableStatus = APSARA_DISK_AVAILABLE_STATUS;
        m_volumeInUseStatus = APSARA_DISK_IN_USE_STATUS;
    }
    virtual ~ApsaraVolumeHandler() {};
    bool CheckAndAttachVolume() override;

    int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 15, uint32_t retryCount = 10) override;

private:
    int32_t DoAttachVolumeToHost(const std::string &volumeId) override;
    int32_t DoCreateNewVolumeFromSnapShotId(const VolSnapInfo &snapshot) override;
    int32_t DoDetachVolumeFromeHost(const std::string &detachVolId, const std::string &serverId) override;
    bool CheckAndDetachVolume(const std::string &volId) override;
    std::string GetProxyHostId() override;

private:
    bool WaitDiskStatus(const Disk &volDetail, std::string &status);
    bool GetDiskDetail(const std::string &volId, Disk &diskDetail);
    bool UpdateNewDiskInfoToFile(Disk &newCreateDisk);
    bool LoadCreatedVolumeInfoFromeFile(const VolSnapInfo &snapshot, Volume &newCreateVolume);
};
}
#endif