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
#ifndef REMOVE_ASSOCIATE_HOST_REQ_H
#define REMOVE_ASSOCIATE_HOST_REQ_H

#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
struct RemoveRelHostReq {
    std::string mHostIdList;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mHostIdList, hostIdList)
    END_SERIAL_MEMEBER
};

class RemoveAssociateHostRequest : public CNwareRequest {
public:
    RemoveAssociateHostRequest() {}
    ~RemoveAssociateHostRequest() {}
 
    void SetRelHostReq(const RemoveRelHostReq relHostReq)
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

    int32_t RemoveAssociateHostRequestToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_relHostReq, m_reqBody)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }
    std::string GetReqBody()
    {
        return m_reqBody;
    }
private:
    RemoveRelHostReq m_relHostReq;
    std::string m_reqBody;
    std::string m_storeId;
};
};
#endif