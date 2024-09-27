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
#ifndef HYPERV_CREATE_SNAPSHOT_MODEL_H
#define HYPERV_CREATE_SNAPSHOT_MODEL_H

namespace HyperVPlugin {
class CreateSnapshotResquest {
public:
	std::string m_vmName;
    std::string m_vmId;
    std::string m_consistencyLevel;
    std::string m_checkPointName;
    std::string m_endPoint;
    std::string m_authKey;
    std::string m_authPwd;
};

class CreateSnapshotResponse {
public:
	std::string m_name;
    std::string m_configDataroot;
    std::string m_instanceId;
    std::string m_configurationFile;
    std::string m_guestStateFile;
    std::string m_configurationID; // ����ID
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, ElementName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configDataroot, ConfigurationDataRoot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_guestStateFile, GuestStateFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceId, InstanceID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configurationFile, ConfigurationFile)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_configurationID, ConfigurationID)
    END_SERIAL_MEMEBER
};
}

#endif