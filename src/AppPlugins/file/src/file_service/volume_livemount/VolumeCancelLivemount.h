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
#ifndef VOLUME_CANCEL_LIVEMOUNT_H
#define VOLUME_CANCEL_LIVEMOUNT_H

#include <string>
#include <vector>
#include "constant/Defines.h"
#include "VolumeCommonService.h"
#include "JsonHelper.h"

namespace FilePlugin {

struct VolumeCancelLivemountFileSystemAdvanceParams {
    std::string shareName;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shareName, shareName)
    END_SERIAL_MEMEBER
};

struct VolumeCancelLivemountFileSystemShareInfo {
    int type;
    std::string fileSystemName;
    int accessPermission;
    VolumeCancelLivemountFileSystemAdvanceParams advanceParams;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemName, fileSystemName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(accessPermission, accessPermission)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(advanceParams, advanceParams)
    END_SERIAL_MEMEBER
};

struct VolumeCancelLivemountDataRepoExtendInfo {
    std::string esn;
    VolumeCancelLivemountFileSystemShareInfo fileSystemShareInfo;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(esn, esn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemShareInfo, fileSystemShareInfo)
    END_SERIAL_MEMEBER
};

class VolumeCancelLivemount : public VolumeCommonService {
public:
    VolumeCancelLivemount() {};

    virtual ~VolumeCancelLivemount() {};

    EXTER_ATTACK int PrerequisiteJob() override;

    EXTER_ATTACK int GenerateSubJob() override;

    EXTER_ATTACK int ExecuteSubJob() override;

    EXTER_ATTACK int PostJob() override;

protected:
    std::shared_ptr<AppProtect::CancelLivemountJob> GetJobInfoBody() const;

    int PrerequisiteJobInner();

    int GenerateSubJobInner();

    int ExecuteSubJobInner();

    int PostJobInner();

    void ReportJob(AppProtect::SubJobStatus::type status);

    virtual bool LoadBasicDirectory();

    void LoadUmountRecords();

    void UmountVolumesFromRecords();

    void UmountVolumeFromRecord(const std::string& mountRecordJsonPath);

    bool SetupCopyRepo();

protected:
    std::shared_ptr<AppProtect::StorageRepository> m_dataRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_cacheRepo = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> m_cancelLivemountPara = nullptr;
    std::string m_cloneCopyId;

    std::string m_nasShareMountTarget;
    std::string m_volumesMountTargetRoot;
    std::string m_volumesMountRecordRoot;
    std::vector<std::string> m_volumeMountRecordJsonList;
    std::string m_shareName;
};
}

#endif