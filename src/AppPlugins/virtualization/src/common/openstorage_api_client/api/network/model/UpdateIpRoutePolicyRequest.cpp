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
#include "UpdateIpRoutePolicyRequest.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

Scope UpdateIpRoutePolicyRequest::GetScopeType() const
{
    return Scope::NONE;
}

ApiType UpdateIpRoutePolicyRequest::GetApiType()
{
    return ApiType::OPENSTORAGE_API;
}

bool UpdateIpRoutePolicyRequest::BuildRequestBody(std::string &body)
{
    RequestBody reqBody;
    reqBody.taskType = m_taskType;
    reqBody.destinationIp = m_destIp;
    std::string reqBodyStr;
    if (!Module::JsonHelper::StructToJsonString(reqBody, reqBodyStr)) {
        ERRLOG("Convert request body to json string failed");
        return false;
    }
    DBGLOG("Update ip route policy body: %s", WIPE_SENSITIVE(reqBodyStr.c_str()));
    body = reqBodyStr;
    return true;
}

void UpdateIpRoutePolicyRequest::SetTaskType(const std::string &type)
{
    m_taskType = type;
}

void UpdateIpRoutePolicyRequest::SetDestIp(const std::string &ip)
{
    m_destIp = ip;
}

std::string UpdateIpRoutePolicyRequest::GetDestIp()
{
    return m_destIp;
}

VIRT_PLUGIN_NAMESPACE_END