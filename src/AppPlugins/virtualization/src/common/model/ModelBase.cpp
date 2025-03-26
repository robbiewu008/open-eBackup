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
#include "ModelBase.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
void ModelBase::SetEnvAddress(const std::string &value)
{
    m_envAddress = value;
    m_envAddressIsSet = true;
}

std::string ModelBase::GetEnvAddress() const
{
    return m_envAddress;
}

bool ModelBase::SetEnvAddressIsSet() const
{
    return m_envAddressIsSet;
}

void ModelBase::SetNeedNewToken()
{
    m_needNewToken = true;
}

bool ModelBase::IsNeedNewToken() const
{
    return m_needNewToken;
}

void ModelBase::SetUserInfo(const AuthObj &value)
{
    m_userInfo = value;
    m_userInfoIsSet = true;
}

AuthObj ModelBase::GetUserInfo() const
{
    return m_userInfo;
}

bool ModelBase::UserInfoIsSet() const
{
    return m_userInfoIsSet;
}

void ModelBase::SetIamUserInfo(const AuthObj &value)
{
    m_iamUserInfo = value;
    m_iamUserInfoIsSet = true;
    INFOLOG("Iam user has set.");
}

AuthObj ModelBase::GetIamUserInfo() const
{
    if (!m_iamUserInfoIsSet) {
        INFOLOG("Iam user has not set, use user info.");
        return m_userInfo;
    }
    INFOLOG("Get iam user info.");
    return m_iamUserInfo;
}

bool ModelBase::IamUserInfoIsSet() const
{
    return m_iamUserInfoIsSet;
}
void ModelBase::SetScopeValue(const std::string &value)
{
    m_scopeValue = value;
    m_scopeValueIsSet = true;
}

std::string ModelBase::GetScopeValue() const
{
    return m_scopeValue;
}

bool ModelBase::ScopeValueIsSet() const
{
    return m_scopeValueIsSet;
}

void ModelBase::SetDomain(const std::string &value)
{
    m_domainName = value;
    m_domainIsSet = true;
}

std::string ModelBase::GetDomain() const
{
    return m_domainName;
}

bool ModelBase::DomainIsSet() const
{
    return m_domainIsSet;
}

void ModelBase::SetEndpoint(const std::string &value)
{
    m_endpoint = value;
    m_endPointIsSet = true;
}

std::string ModelBase::GetEndpoint() const
{
    return m_endpoint;
}

bool ModelBase::EndPointIsSet() const
{
    return m_endPointIsSet;
}

void ModelBase::SetRegion(const std::string &value)
{
    m_region = value;
    m_regionIsSet = true;
}

std::string ModelBase::GetRegion() const
{
    return m_region;
}

bool ModelBase::SetRegionIsSet() const
{
    return m_regionIsSet;
}

void ModelBase::SetNeedRetry(const bool &value)
{
    m_needRetry = value;
}

bool ModelBase::GetNeedRetry() const
{
    return m_needRetry;
}

void ModelBase::SetDomainId(const std::string &value)
{
    m_domainId = value;
}

std::string ModelBase::GetDomainId() const
{
    return m_domainId;
}

void ModelBase::SetToken(const std::string &value)
{
    m_token = value;
    m_tokenIsSet = true;
}

std::string ModelBase::GetToken() const
{
    return m_token;
}

void ModelBase::SetExtendInfo(const std::string &value)
{
    m_extendInfo = value;
}

std::string ModelBase::GetExtendInfo() const
{
    return m_extendInfo;
}
}