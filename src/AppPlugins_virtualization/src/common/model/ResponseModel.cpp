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
#include "ResponseModel.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
bool ResponseModel::Success() const
{
    return m_success;
}

bool ResponseModel::Busy(void) const
{
    return m_busy;
}

uint32_t  ResponseModel::GetHttpStatusCode() const
{
    return m_httpStatusCode;
}

std::string ResponseModel::GetHttpStatusDescribe() const
{
    return m_httpStatusDescribe;
}

int32_t ResponseModel::GetErrCode() const
{
    return m_errCode;
}

std::string ResponseModel::GetErrString() const
{
    return m_errString;
}

std::string ResponseModel::GetBody() const
{
    return m_body;
}

std::map<std::string, std::set<std::string> > ResponseModel::GetHeaders() const
{
    return m_headers;
}

std::set<std::string> ResponseModel::GetCookies() const
{
    return m_cookies;
}

uint32_t ResponseModel::GetStatusCode() const
{
    return m_statusCode;
}

void ResponseModel::SetSuccess(const bool &value)
{
    m_success = value;
}

void ResponseModel::SetBusy(const bool &value)
{
    m_busy = value;
}

void ResponseModel::SetHttpStatusCode(const uint32_t &value)
{
    m_httpStatusCode = value;
}

void ResponseModel::SetHttpStatusDescribe(const std::string &value)
{
    m_httpStatusDescribe = value;
}

void ResponseModel::SetErrCode(const int32_t &value)
{
    m_errCode = value;
}

void ResponseModel::SetErrString(const std::string &value)
{
    m_errString = value;
}

void ResponseModel::SetGetBody(const std::string &value)
{
    m_body = value;
}

void ResponseModel::SetHeaders(const std::map<std::string, std::set<std::string> > &value)
{
    m_headers = value;
}

void ResponseModel::SetCookies(const std::set<std::string> &value)
{
    m_cookies = value;
}

void ResponseModel::SetStatusCode(const uint32_t &value)
{
    m_statusCode = value;
}
VIRT_PLUGIN_NAMESPACE_END