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
#include "CNwareRequest.h"

namespace CNwarePlugin {

Scope CNwareRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType CNwareRequest::GetApiType()
{
    return ApiType::CNWARE;
}

std::string CNwareRequest::GetBody()
{
    return m_body;
}

void CNwareRequest::SetIpPort(std::string port)
{
    m_port = port;
    return;
}

std::string CNwareRequest::GetPort()
{
    return m_port;
}

bool CNwareRequest::BuildBody(std::string &body)
{
    m_body = body;
    return true;
}

void CNwareRequest::SetResourceId(const std::string &id)
{
    m_id = id;
    return;
}

std::string CNwareRequest::GetResourceId()
{
    return m_id;
}
}