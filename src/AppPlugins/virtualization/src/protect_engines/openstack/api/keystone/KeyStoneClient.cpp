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
#include "KeyStoneClient.h"
#include "log/Log.h"
#include "config_reader/ConfigIniReader.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainProjectsRequest.h"
#include "protect_engines/openstack/api/keystone/model/GetDomainProjectsResponse.h"


namespace {
using Defer = std::shared_ptr<void>;
}

namespace OpenStackPlugin {
bool KeyStoneClient::CheckParams(ModelBase &model)
{
    if (!model.UserInfoIsSet()) {
        ERRLOG("The param user not be set.");
        return false;
    }
    
    if (!model.EndPointIsSet()) {
        ERRLOG("The param endpoint not be set.");
        return false;
    }
    return true;
}

Json::Value KeyStoneClient::GenDomain(const GetTokenRequest &request)
{
    Json::Value domain;
    std::string domainName;
    Scope scopeType = request.GetScopeType();
    if (scopeType == Scope::ADMIN_PROJECT) {
        domainName = Module::ConfigReader::getString("OpenStackConfig", "AdminRoleDomain");
    } else {
        domainName = request.GetDomain();
    }
    if (domainName.empty()) {
        domain["id"] = request.GetDomainId();
    } else {
        domain["name"] = domainName;
    }
    return domain;
}

Json::Value KeyStoneClient::GenUserInfo(const GetTokenRequest &request)
{
    Json::Value userInfo;
    userInfo["name"] = request.GetUserInfo().name;
    userInfo["password"] = request.GetUserInfo().passwd;
    userInfo["domain"] = GenDomain(request);
    return userInfo;
}

Json::Value KeyStoneClient::GenPwdInfo(const GetTokenRequest &request)
{
    Json::Value passwd;
    passwd["user"] = GenUserInfo(request);
    return passwd;
}

Json::Value KeyStoneClient::GenIdentity(const GetTokenRequest &request)
{
    Json::Value identity;
    identity["methods"].append("password");
    identity["password"] = GenPwdInfo(request);
    return identity;
}

Json::Value KeyStoneClient::GenAuth(const GetTokenRequest &request)
{
    Json::Value auth;
    auth["identity"] = GenIdentity(request);
    if (request.GetScopeType() != Scope::NONE) {
        auth["scope"] = GenScope(request);
    }
    return auth;
}

Json::Value KeyStoneClient::GenProject(const GetTokenRequest &request)
{
    Json::Value project;
    project["domain"] = GenDomain(request);
    if (request.GetScopeType() == Scope::PROJECT) {
        project["id"] = request.GetScopeValue();
    } else {
        project["name"] = Module::ConfigReader::getString("OpenStackConfig", "AdminRoleProject");
    }
    return project;
}

Json::Value KeyStoneClient::GenScope(const GetTokenRequest &request)
{
    Json::Value scope;
    scope["project"] = GenProject(request);
    return scope;
}

std::string KeyStoneClient::BuildQueryBody(const GetTokenRequest &request)
{
    Json::Value jsonReq;
    jsonReq["auth"] = GenAuth(request);
    Json::FastWriter fastWriter;
    std::string body = fastWriter.write(std::move(jsonReq));
    return std::move(body);
}

std::shared_ptr<GetTokenResponse> KeyStoneClient::GetToken(GetTokenRequest &request)
{
    if (!CheckParams(request)) {
        ERRLOG("Failed to check param.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "POST";
    requestInfo.m_resourcePath = "{keystoneAddr}/auth/tokens";
    requestInfo.m_pathParams["keystoneAddr"] = request.GetEnvAddress();
    requestInfo.m_queryParams = {};

    requestInfo.m_body = BuildQueryBody(request);
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetTokenResponse> response = std::make_shared<GetTokenResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Failed to send request.");
        return response;       // cert error, CallApi will return FALSE, this scene need judge err code.
    }
    return response;
}

std::shared_ptr<GetDomainsResponse> KeyStoneClient::GetDomains(GetDomainsRequest &request)
{
    std::string endpoint;
    std::string token;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(token); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, token, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{keystoneAddr}/domains";
    requestInfo.m_pathParams["keystoneAddr"] = request.GetEnvAddress();
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(token);
    requestInfo.m_body = "";
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetDomainsResponse> response = std::make_shared<GetDomainsResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Failed to send request get domains.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetDomainProjectsResponse> KeyStoneClient::GetDomainProjects(GetDomainProjectsRequest &request)
{
    std::string endpoint;
    std::string token;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(token); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, token, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{keystoneAddr}/projects";
    requestInfo.m_pathParams["keystoneAddr"] = request.GetEnvAddress();
    requestInfo.m_queryParams["domain_id"] = request.GetDomainId();
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(token);
    requestInfo.m_body = "";
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetDomainProjectsResponse> response = std::make_shared<GetDomainProjectsResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Failed to send request.");
        return nullptr;
    }
    return response;
}

std::shared_ptr<GetServicesResponse> KeyStoneClient::GetServices(GetServicesRequest &request)
{
    std::string endpoint;
    std::string token;
    Defer _(nullptr, [&](...) { Module::CleanMemoryPwd(token); });
    if (!OpenStackTokenMgr::GetInstance().GetToken(request, token, endpoint)) {
        ERRLOG("Get token failed.");
        return nullptr;
    }
    RequestInfo requestInfo;
    requestInfo.m_method = "GET";
    requestInfo.m_resourcePath = "{keystoneAddr}/services";
    requestInfo.m_pathParams["keystoneAddr"] = request.GetEnvAddress();
    if (!request.GetServiceName().empty()) {
        requestInfo.m_queryParams["name"] = request.GetServiceName();
    }
    requestInfo.m_headerParams["X-Auth-Token"] = std::move(token);
    requestInfo.m_body = "";
    requestInfo.m_auth = request.GetUserInfo();
    std::shared_ptr<GetServicesResponse> response = std::make_shared<GetServicesResponse>();
    if (CallApi(requestInfo, response, request) != SUCCESS) {
        ERRLOG("Failed to send request.");
        return nullptr;
    }
    if (response->GetStatusCode() != static_cast<uint32_t>(Module::SC_OK)) {
        ERRLOG("Failed to get services, status: %d, resp body: %s.", response->GetStatusCode(),
            response->GetBody().c_str());
        return response;
    }
    if (!response->Serial()) {
        ERRLOG("Failed to serial response body.");
        return nullptr;
    }
    return response;
}

bool KeyStoneClient::UpdateToken(ModelBase &request, std::string &tokenStr)
{
    if (!OpenStackTokenMgr::GetInstance().ReacquireToken(request, tokenStr)) {
        ERRLOG("Get token failed.");
        return false;
    }
    return true;
}
}
