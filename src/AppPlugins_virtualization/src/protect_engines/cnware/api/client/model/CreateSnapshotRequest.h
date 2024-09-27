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
#ifndef CREATE_SNAPSHOT_REQ_H
#define CREATE_SNAPSHOT_REQ_H
 
#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {

struct CustomDomainSnapshotReq {
    std::string m_description = "";
    std::vector<std::string> m_snapDisks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapDisks, snapDisks)
    END_SERIAL_MEMEBER
};

class CreateSnapshotRequest : public CNwareRequest {
public:
    CreateSnapshotRequest() {}
    ~CreateSnapshotRequest() {}
 
    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }

    int32_t CustomDomainSnapshotReqToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_customDomainSnapshotReq, m_body)) {
            ERRLOG("Convert snap info to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

    void SetCustomDomainSnapshotReq(const CustomDomainSnapshotReq &customDomainSnapshotReq)
    {
        m_customDomainSnapshotReq = customDomainSnapshotReq;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }

private:
    CustomDomainSnapshotReq m_customDomainSnapshotReq;
    std::string m_domainId;
};
};
 
#endif