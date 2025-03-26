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
#ifndef MODELBASE_H
#define MODELBASE_H
#include "common/token_mgr/TokenDetail.h"
#include "json/json.h"
#include "common/Macros.h"
#include "common/Structs.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
class ModelBase {
public:
    ModelBase() {}
    virtual ~ModelBase() {}

    // 设置scopeValue类型
    virtual Scope GetScopeType() const = 0;

    // 设置api类型
    virtual ApiType GetApiType() = 0;

    void SetApiType(ApiType type)
    {
        m_apiType = type;
    };

    void SetScope(const Scope type)
    {
        m_scope = type;
    };

    // 平台ip,用户生成管理token的key
    void SetEnvAddress(const std::string &value);
    std::string GetEnvAddress() const;
    bool SetEnvAddressIsSet() const;

    // 不使用缓存中的token, 默认使用缓存的token
    void SetNeedNewToken();
    bool IsNeedNewToken() const;

    // 获取token时，用户的鉴权信息
    void SetUserInfo(const AuthObj &value);
    AuthObj GetUserInfo() const;
    bool UserInfoIsSet() const;

    // 获取token时，iam用户的鉴权信息
    void SetIamUserInfo(const AuthObj &value);
    AuthObj GetIamUserInfo() const;
    bool IamUserInfoIsSet() const;

    // 获取token时，用户域名
    void SetDomain(const std::string &value);
    std::string GetDomain() const;
    bool DomainIsSet() const;

    // 获取token时，ScopeValue
    void SetScopeValue(const std::string &value);
    std::string GetScopeValue() const;
    bool ScopeValueIsSet() const;

    // iam 和 sc 接口组装url路径，由环境配置文件获取并注册到平台上，一般为 demo.com
    void SetEndpoint(const std::string &value);
    std::string GetEndpoint() const;
    bool EndPointIsSet() const;

    // region: 用于获取指定域名的endpoint，主要用于evs和ecs接口
    void SetRegion(const std::string &value);
    std::string GetRegion() const;
    bool SetRegionIsSet() const;

    void SetNeedRetry(const bool &value);
    bool GetNeedRetry() const;

    void SetDomainId(const std::string &value);
    std::string GetDomainId() const;

    void SetToken(const std::string &value);
    std::string GetToken() const;

    void SetExtendInfo(const std::string &value);
    std::string GetExtendInfo() const;
    
protected:
    std::string m_envAddress;
    bool m_envAddressIsSet = false;

    std::string m_domainName;
    bool m_domainIsSet = false;

    std::string m_endpoint;
    bool m_endPointIsSet = false;

    AuthObj m_userInfo;
    bool m_userInfoIsSet = false;

    AuthObj m_iamUserInfo;
    bool m_iamUserInfoIsSet = false;

    std::string m_scopeValue;
    bool m_scopeValueIsSet = false;

    std::string m_region;
    bool m_regionIsSet = false;

    bool m_needRetry = true;

    std::string m_domainId;

    std::string m_token = "";
    bool m_tokenIsSet = false;

    bool m_needNewToken = false;

    ApiType m_apiType;

    Scope m_scope;

    std::string m_extendInfo = "";
};
}

#endif