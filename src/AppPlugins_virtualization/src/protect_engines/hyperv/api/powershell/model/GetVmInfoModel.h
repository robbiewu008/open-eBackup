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
#ifndef HYPERV_GET_VM_INFO_MODEL_H
#define HYPERV_GET_VM_INFO_MODEL_H

namespace HyperVPlugin {

class GetVMInfoResponse {
public:
    std::string m_id;
    std::string m_configurationLocation;
    std::string m_checkpointFileLocation;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, Id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configurationLocation, ConfigurationLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_checkpointFileLocation, CheckpointFileLocation)
    END_SERIAL_MEMEBER
};

class GetVMInfoRequest {
public:
    std::string m_vmId;
    std::string m_endPoint;
    std::string m_authKey;
    std::string m_authPwd;
};
}
#endif