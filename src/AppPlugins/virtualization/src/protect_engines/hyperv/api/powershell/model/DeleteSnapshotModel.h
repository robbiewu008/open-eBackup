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
#ifndef _HYPERV_DELETE_SNAPSHOT_MODEL_H_
#define _HYPERV_DELETE_SNAPSHOT_MODEL_H_

#include "common/CleanMemPwd.h"

namespace HyperVPlugin {
class DeleteSnapshotResquest {
public:
	std::string m_vmName;
    std::string m_vmId;
    std::string m_checkPointName;
    std::string m_checkPointInstanceId;
    std::string m_authKey;
    std::string m_authPwd;

    ~DeleteSnapshotResquest()
    {
        Module::CleanMemoryPwd(m_authPwd);
    }
};

class DeleteSnapshotResponse {
public:
    std::string m_name;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, ElementName)
    END_SERIAL_MEMEBER
};
}

#endif