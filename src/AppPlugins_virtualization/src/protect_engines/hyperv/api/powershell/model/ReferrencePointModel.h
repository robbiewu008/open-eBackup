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
#ifndef HYPERV_REFERRENCE_POINT_MODEL_H
#define HYPERV_REFERRENCE_POINT_MODEL_H

namespace HyperVPlugin {
class ReferrencePointResquest {
public:
    std::string m_vmId;
    std::string m_checkPointName;
    std::string m_endPoint;
    std::string m_authKey;
    std::string m_authPwd;
    std::string m_instanceId;
};

class ReferrencePointResponse {
public:
    std::string m_name;
    std::string m_consistencyLevel;
    std::string m_instanceId;
    std::string m_referencePointType;
    std::vector<std::string> m_rctList;
    std::vector<std::string> m_virtualDiskIdentifiers;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, ElementName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistencyLevel, ConsistencyLevel)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceId, InstanceID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_referencePointType, ReferencePointType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_rctList, ResilientChangeTrackingIdentifiers)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_virtualDiskIdentifiers, VirtualDiskIdentifiers)
    END_SERIAL_MEMEBER
};
}

#endif // HYPERV_REFERRENCE_POINT_MODEL_H