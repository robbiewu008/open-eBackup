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
#ifndef PS_CLIENT_H
#define PS_CLIENT_H

#include <json/json.h>
#include "common/JsonHelper.h"
#include "log/Log.h"
#include "protect_engines/hyperv/api/powershell/model/CreateSnapshotModel.h"
#include "protect_engines/hyperv/api/powershell/model/GetVMVolumesModel.h"
#include "protect_engines/hyperv/api/powershell/model/ReferrencePointModel.h"
#include "protect_engines/hyperv/api/powershell/model/GetVmInfoModel.h"
#include "protect_engines/hyperv/api/powershell/model/GetVmHardDiskDriveModel.h"
#include "protect_engines/hyperv/api/powershell/model/CreateVHDModel.h"
#include "protect_engines/hyperv/api/powershell/model/DeleteSnapshotModel.h"

using namespace HyperVPlugin;
namespace HyperVPlugin {

struct ActionResultRes {
    int32_t m_code = 0;
    int64_t m_bodyErr = 0;
    std::string m_message;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_code, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bodyErr, bodyErr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_message, message)
    END_SERIAL_MEMEBER
};

class Auth {
public:
    std::string m_authKey;
    std::string m_authPwd;

    ~Auth()
    {
        Module::CleanMemoryPwd(m_authPwd);
    }
};

class PSClient {
public:
    PSClient() {};
    ~PSClient() {};
    std::shared_ptr<CreateSnapshotResponse> CreateCheckPoint(CreateSnapshotResquest &request);
    std::shared_ptr<GetVMVolumesResponse> GetVMVolumes(GetVMVolumesRequest &request);
    std::shared_ptr<ReferrencePointResponse> ConvertCheckPoint(ReferrencePointResquest &request);
    std::shared_ptr<GetVMInfoResponse> GetVmInfo(GetVMInfoRequest &request);
    std::shared_ptr<GetVMHardDiskDriveResponse> GetVmDrive(GetVMHardDiskDriveRequest &request);
    bool DeleteReferPointExcept(ReferrencePointResquest &request);
    std::shared_ptr<CreateVHDResponse> CreateVHD(CreateVHDRequest &request);
    bool DeleteSpecificSnapshot(DeleteSnapshotResquest &request);

private:
    template<typename T>
    ActionResultRes Executor(const std::string &command, T &res, Auth &auth,
        const Json::Value &param = Json::Value());
    bool CheckResultValid(const Json::Value &result);
};
}

#endif // PS_CLIENT_H