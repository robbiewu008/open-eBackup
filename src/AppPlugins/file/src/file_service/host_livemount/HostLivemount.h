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
#ifndef HOSTLIVEMOUNT_H
#define HOSTLIVEMOUNT_H
 
#include <memory>
#include "BasicJob.h"
#include "Module/src/common/JsonHelper.h"
#include "client/ClientInvoke.h"
#ifdef WIN32
#include "constant/Defines.h"
#endif
namespace FilePlugin {
struct HostLivemountFileSystemShareInfoAdvanceParam {
    std::string clientName;
    int clientType {0};
    int portSecure {0};
    int rootSquash {0};
    int squash {0};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clientName, clientName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(clientType, clientType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(portSecure, portSecure)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(rootSquash, rootSquash)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(squash, squash)
    END_SERIAL_MEMEBER
};
 
struct HostLivemountFileSystemShareInfo {
    int accessPermission {0};
    HostLivemountFileSystemShareInfoAdvanceParam advanceParams;
    std::string fileSystemName {0};
    int type {0};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(accessPermission, accessPermission)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(advanceParams, advanceParams)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemName, fileSystemName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(type, type)
    END_SERIAL_MEMEBER
};
 
struct FileSetMountDrive {
    std::string mountDirve;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountDirve, MountDirve)
    END_SERIAL_MEMEBER
};

struct HostLivemountCopyExtendAddDrive {
    int dataAfterReduction;
    int dataBeforeReduction;
    std::string dataPathSuffix;
    std::string isAggregation;
    std::string maxSizeAfterAggregate;
    std::string maxSizeToAggregate;
    std::string metaPathSuffix;
    std::string multiFileSystem;
    std::string driveInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataAfterReduction, dataAfterReduction)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataBeforeReduction, dataBeforeReduction)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dataPathSuffix, dataPathSuffix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isAggregation, isAggregation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxSizeAfterAggregate, maxSizeAfterAggregate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(maxSizeToAggregate, maxSizeToAggregate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaPathSuffix, metaPathSuffix)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(multiFileSystem, multiFileSystem)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(driveInfo, driveInfo)
    END_SERIAL_MEMEBER
};

struct HostLivemountExtend {
    std::string dstPath;
    std::vector<HostLivemountFileSystemShareInfo> fileSystemShareInfo;
    std::string fibreChannel;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(dstPath, dstPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileSystemShareInfo, fileSystemShareInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fibreChannel, fibreChannel)
    END_SERIAL_MEMEBER
};

class HostLivemount : public BasicJob {
public:
    HostLivemount() {};
    ~HostLivemount() {};
 
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;
 
private:
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int AddDriveNameToExtendInfo(const std::string& driveName);
    int GetFileSetMountDriveInfo(ActionResult &actionResult);
    int ExecuteSubJobInner();
    int PostJobInner();
    int IdentifyRepos();
    void ReportJobDetailsWithLabelAndErrcode(
        const std::tuple<JobLogLevel::type, SubJobStatus::type, const int>& reportInfo,
        const std::string& logLabel, const int64_t errCode, const std::string& message);
    void ReportJob(AppProtect::SubJobStatus::type status);
    std::shared_ptr<AppProtect::LivemountJob> GetJobInfoBody();
    int IdentifyDataRepo();
 
    std::shared_ptr<AppProtect::StorageRepository> m_dataRepo = nullptr;
    std::shared_ptr<AppProtect::LivemountJob> m_livemountPara = nullptr;
};
}
#endif