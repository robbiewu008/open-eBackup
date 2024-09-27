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
#ifndef ASSOCIATE_HOST_REQ_H
#define ASSOCIATE_HOST_REQ_H
 
#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
struct RelHostReq {
    std::string mHostIdList;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostIdList, hostIdList)
    END_SERIAL_MEMEBER
};
 
class AssociateHostRequest : public CNwareRequest {
public:
    AssociateHostRequest() {}
    ~AssociateHostRequest() {}
 
    void SetRelHostReq(const RelHostReq relHostReq)
    {
        m_relHostReq = relHostReq;
    }

    void SetStoreId(const std::string &storeId)
    {
        m_storeId = storeId;
    }
    
    std::string GetStoreId()
    {
        return m_storeId;
    }

    int32_t AssociateHostRequestToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_relHostReq, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

private:
    RelHostReq m_relHostReq;
    std::string m_storeId;
};
};
#endif