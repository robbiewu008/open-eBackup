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
#include "GetPoolsResponse.h"
#include "common/utils/Utils.h"
 
OPENSTACK_PLUGIN_NAMESPACE_BEGIN
 
GetPoolsResponse::GetPoolsResponse() {}
GetPoolsResponse::~GetPoolsResponse() {}
 
bool GetPoolsResponse::Serial()
{
    if (m_body.empty()) {
        ERRLOG("GetPoolsResponse body is empty.");
        return false;
    }
    Json::Value bodyVaule;
    if (!Module::JsonHelper::JsonStringToJsonValue(m_body, bodyVaule)) {
        ERRLOG("JsonStringToJsonValue error, str: %s", m_body.c_str());
        return false;
    }
    for (auto &pool : bodyVaule["pools"]) {
        if (pool["capabilities"].isMember("total_capacity_gb") &&
            pool["capabilities"]["total_capacity_gb"].isString()) {
            pool["capabilities"]["total_capacity_gb"] =
                VirtPlugin::Utils::SafeStod(pool["capabilities"]["total_capacity_gb"].asString());
        }
    }
    if (!Module::JsonHelper::JsonValueToStruct(bodyVaule, m_pools)) {
        ERRLOG("Failed to convert jsonvalue storagePool to Struct.");
        return false;
    }
    return true;
}
 
Pools GetPoolsResponse::GetPools() const
{
    return m_pools;
}
 
OPENSTACK_PLUGIN_NAMESPACE_END