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
#ifndef HOSTCANCELLIVEMOUNT_H
#define HOSTCANCELLIVEMOUNT_H
 
#include <memory>
#include "BasicJob.h"
#ifdef WIN32
#include "Defines.h"
#endif
#include "FsDevice.h"
#include "DeviceMount.h"
#include "client/ClientInvoke.h"

namespace FilePlugin {
class HostCancelLivemount : public BasicJob {
public:
    HostCancelLivemount() {};
    ~HostCancelLivemount() {};

    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;

private:
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int GetCmdArgForWin32(std::string& exeCmd);
    int GetCmdArgForLinux(std::string& exeCmd);
    int ExecuteSubJobInner();
    int PostJobInner();
    int IdentifyRepos();
    bool ExecuteUmountCmd(const std::string& mp);
    bool CheckMountPointExists(const std::string& mp);
    void ReportJob(AppProtect::SubJobStatus::type status);
    bool CheckBlackList(const std::string& dir) const;
    std::shared_ptr<AppProtect::CancelLivemountJob> GetJobInfoBody();
    int IdentifyDataRepo();
    std::shared_ptr<AppProtect::StorageRepository> m_dataRepo = nullptr;
    std::shared_ptr<AppProtect::CancelLivemountJob> m_cancelLivemountPara = nullptr;
    std::shared_ptr<FilePlugin::DeviceMount> m_deviceMountPtr {nullptr};
};
}

#endif