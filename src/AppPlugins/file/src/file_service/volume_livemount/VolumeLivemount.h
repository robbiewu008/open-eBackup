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
#ifndef VOLUME_LIVEMOUNT_H
#define VOLUME_LIVEMOUNT_H

#include <memory>
#include <string>
#include "constant/Defines.h"
#include "volume/VolumeCommonService.h"

namespace FilePlugin {

class VolumeLivemount : public VolumeCommonService {
public:
    VolumeLivemount() {};

    virtual ~VolumeLivemount() {};

    EXTER_ATTACK int PrerequisiteJob() override;

    EXTER_ATTACK int GenerateSubJob() override;

    EXTER_ATTACK int ExecuteSubJob() override;

    EXTER_ATTACK int PostJob() override;

protected:
    virtual bool MountShare();

    std::shared_ptr<AppProtect::LivemountJob> GetJobInfoBody() const;

    int PrerequisiteJobInner();

    int GenerateSubJobInner();

    int ExecuteSubJobInner();

    int PostJobInner();

    void ReportJob(AppProtect::SubJobStatus::type status);

    bool SetupCopyRepo();

    virtual bool PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo);

    virtual bool MountVolumes();

    bool MountSingleVolumeCopy(const std::string& volumeName, const std::string& volumeDirPath);

protected:
    std::shared_ptr<AppProtect::StorageRepository> m_dataRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_cacheRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_metaRepo = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> m_livemountPara = nullptr;
    std::string m_cloneCopyId;
    std::string m_nasShareMountTarget;
    std::string m_volumesMountTargetRoot;
    std::string m_volumesMountRecordRoot;
    std::string m_dataPathRoot;

    std::vector<std::string> m_mountedRecords;
};

}

#endif