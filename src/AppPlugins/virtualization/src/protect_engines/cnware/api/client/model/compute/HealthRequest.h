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
#ifndef __HEALTH_REQUEST__
#define __HEALTH_REQUEST__

#include <string>
#include "protect_engines/cnware/common/Structs.h"
#include "../CNwareRequest.h"

namespace CNwarePlugin {

struct HealthVmReq {
    std::vector<std::string> domainIds;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(domainIds)
    END_SERIAL_MEMEBER
};

class HealthRequest : public CNwareRequest {
public:
    HealthRequest() {}
    ~HealthRequest() {}

    void AddToHealthDomain(const std::string &doaminId)
    {
        m_healthList.domainIds.emplace_back(doaminId);
    }

    int32_t BuildRequestBodyString()
    {
        if (!Module::JsonHelper::StructToJsonString(m_healthList, m_body)) {
            ERRLOG("Convert healthList to json string failed!");
            return VirtPlugin::FAILED;
        }
        DBGLOG("Convert to string: %s", WIPE_SENSITIVE(m_body).c_str());
        return VirtPlugin::SUCCESS;
    }

private:
    HealthVmReq m_healthList;
};
};

#endif