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
#ifndef VIR_GET_TOKEN_DETAIL_H
#define VIR_GET_TOKEN_DETAIL_H

#include <vector>
#include <map>
#include <common/JsonHelper.h>
#include "common/Macros.h"
#include "common/CleanMemPwd.h"


VIRT_PLUGIN_NAMESPACE_BEGIN
const std::string HTTP_RESPONSE_HEADER_TOKNE_FILED_NAME = "X-Auth-Token";
const std::string AUTHORIZATION_HTTP_RESPONSE_HEADER_TOKNE_FILED_NAME = "Authorization";
struct Domain {
    std::string m_name;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

struct User {
    Domain m_domain;
    std::string m_name;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domain, domain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

struct TokenProject {
    Domain m_domain;
    std::string m_name;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_domain, domain)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

struct Role {
    std::string m_name;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

struct Endpoint {
    std::string m_regionId;
    std::string m_id;
    std::string m_region;
    std::string m_interface;
    std::string m_url;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, region_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_region, region)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_interface, interface)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_url, url)
    END_SERIAL_MEMEBER
};

struct Catalog {
    std::vector<Endpoint> m_endpoints;
    std::string m_name;
    std::string m_id;
    std::string m_type;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_endpoints, endpoints)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    END_SERIAL_MEMEBER
};

struct Token {
    std::string m_expiresAt;
    std::vector<std::string> m_methods;
    std::vector<Catalog> m_catalog;
    std::vector<Role> m_roles;
    TokenProject m_project;
    std::string issuedAt;
    User m_user;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_expiresAt, expires_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_methods, methods)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_catalog, catalog)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_roles, roles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_project, project)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(issuedAt, issued_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_user, user)
    END_SERIAL_MEMEBER
};

struct TokenDetail {
    Token m_token;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_token, token)
    END_SERIAL_MEMEBER
};

struct TokenInfo {
    std::string m_token;             // token值
    std::string m_expiresDate;       // token过期时间
    std::string m_extendInfo;        // 获取保存token后的返回值

    ~TokenInfo()
    {
        Module::CleanMemoryPwd(m_token);
    }
};

enum class Scope {
    NONE, // 没有scope
    USER_DOMAIN, // 作用域
    PROJECT, // 工程
    ADMIN_PROJECT, // 管理员工程
    AUTHORIZATION
};

enum class ApiType {
    IAM,    // IAM对外访问简称
    SC,     // 运营侧对外访问简称
    ECS,    // 计算资源简称
    EVS,    // 存储资源简称
    CINDER,  // Cinder资源
    NOVA,
    FUSION_STORAGE,
    KEYSTONE,
    NEUTRON,
    FUSIONSTORAGE,
    CNWARE,
    OPENSTORAGE_API,
    NUTANIX
};

VIRT_PLUGIN_NAMESPACE_END

#endif