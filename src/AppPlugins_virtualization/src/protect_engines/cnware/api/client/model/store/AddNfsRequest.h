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
#ifndef ADD_NFS_REQUEST_H
#define ADD_NFS_REQUEST_H

#include <string>
#include "../CNwareRequest.h"
#include "common/Constants.h"

namespace CNwarePlugin {

class AddNfsRequest : public CNwareRequest {
public:
    AddNfsRequest() {}
    ~AddNfsRequest() {}

    bool SetNfsReq(const std::string &name, const std::string &hostId,
        const std::string &ip)
    {
        AddNfsReq addNfsReq;
        addNfsReq.m_name = name;
        addNfsReq.m_hostIdList.emplace_back(hostId);
        addNfsReq.m_ip = ip;
        if (!Module::JsonHelper::StructToJsonString(addNfsReq, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return false;
        }
        return true;
    }
};
};
#endif