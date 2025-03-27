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
#include "NutanixRequest.h"

namespace NutanixPlugin {

Scope NutanixRequest::GetScopeType() const
{
    return Scope::PROJECT;
}

ApiType NutanixRequest::GetApiType()
{
    return ApiType::NUTANIX;
}

std::string NutanixRequest::GetBody()
{
    return m_body;
}

void NutanixRequest::SetIpPort(std::string port)
{
    m_port = port;
    return;
}

std::string NutanixRequest::GetPort() const
{
    return m_port;
}

bool NutanixRequest::BuildBody(std::string &body)
{
    m_body = body;
    return true;
}

void NutanixRequest::SetResourceId(const std::string &id)
{
    m_id = id;
    return;
}

std::string NutanixRequest::GetResourceId()
{
    return m_id;
}
}