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
#ifndef __QUERY_HOSTLIST_REQUEST__
#define __QUERY_HOSTLIST_REQUEST__

#include <string>
#include "protect_engines/cnware/common/Structs.h"
#include "../CNwareRequest.h"

namespace CNwarePlugin {
class QueryHostListRequest : public CNwareRequest {
public:
    QueryHostListRequest() {}
    ~QueryHostListRequest() {}

    void SetHostIds(const std::vector<std::string> &hostIds)
    {
        m_hostIds = hostIds;
    }
    std::vector<std::string> GetHostIds()
    {
        return m_hostIds;
    }
    std::string GetHostIdsString()
    {
        std::string hostIds;
        for (auto hostId : m_hostIds) {
            if (!hostIds.empty()) {
                hostIds += ",";
            }
            hostIds += hostId;
        }
        return hostIds;
    }
private:
    std::vector<std::string> m_hostIds;
};
};
#endif