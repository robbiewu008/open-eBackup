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
#include "OpenStackTokenMgr.h"
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "common/token_mgr/TokenDetail.h"
#include "protect_engines/openstack/api/keystone/KeyStoneClient.h"
#include "protect_engines/openstack/api/keystone/model/GetTokenRequest.h"
#include "log/Log.h"
#include "config_reader/ConfigIniReader.h"


using namespace std;
using VirtPlugin::ApiType;
using VirtPlugin::TokenDetail;
using VirtPlugin::Catalog;
using VirtPlugin::Endpoint;

namespace {
const std::string MODULE_NAME = "OpenStackTokenMgr";
const std::string KEY_TOKEN_PREFIX_DOMAIN = "_domain_";
const std::string KEY_TOKEN_PREFIX_PROJECT = "_project_";
const int ONE_HOUR = 3600;
}

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

static std::once_flag startCheckTokenThread;

OpenStackTokenMgr& OpenStackTokenMgr::GetInstance()
{
    static OpenStackTokenMgr instance;
    std::call_once(startCheckTokenThread, [&]() {
        std::thread thread(&OpenStackTokenMgr::CheckToken, &instance);
        thread.detach();
    });
    return instance;
}

/**
 * @brief 基于admin project key: https://identity.az236.dc236.huawei.com:443/identity/v3_project_admin
 * 基于业务project key: https://identity.az236.dc236.huawei.com:443/identity/v3_project_0e369b845ef142a7b189e98913f8233a
 * @param model
 * @return std::string
 */
std::string OpenStackTokenMgr::GetTokenKey(ModelBase &model)
{
    std::string key;
    if (model.GetScopeType() == Scope::PROJECT) {
        key = model.GetEnvAddress() + std::string(KEY_TOKEN_PREFIX_PROJECT) + model.GetScopeValue();
    } else if (model.GetScopeType() == Scope::ADMIN_PROJECT) {
        std::string project = Module::ConfigReader::getString("OpenStackConfig", "AdminRoleProject");
        key = model.GetEnvAddress() + std::string(KEY_TOKEN_PREFIX_PROJECT) + project;
    } else if (model.GetScopeType() == Scope::NONE) {
        key = model.GetEnvAddress() + std::string(KEY_TOKEN_PREFIX_DOMAIN) + model.GetDomain();
    } else {
        ERRLOG("invalid scope Type.");
    }
    
    return key;
}

/**
 * @brief 获取当前系统时间与GMT时间的差异
 *
 * @return int
 */
int OpenStackTokenMgr::GetTimeDiffWithGMT()
{
    time_t systemTime = time(NULL);
    tm gmtTime = *gmtime(&systemTime); // 系统时间转换为GMT时间
    time_t gmtSystemTime = mktime(&gmtTime); // 再将GMT时间重新转换为系统时间
    tm gmtTimeObj = *localtime(&gmtSystemTime);
    return systemTime - gmtSystemTime + (gmtTimeObj.tm_isdst ? ONE_HOUR : 0);
}

bool OpenStackTokenMgr::ReacquireToken(ModelBase &model, std::string &tokenValue)
{
    std::lock_guard<std::mutex> locker(m_tokenMutex);
    GetTokenRequest getTokenRequeset;
    getTokenRequeset.SetScopeType(model.GetScopeType());
    getTokenRequeset.SetScopeValue(model.GetScopeValue());
    getTokenRequeset.SetUserInfo(model.GetUserInfo());
    getTokenRequeset.SetDomain(model.GetDomain());
    getTokenRequeset.SetDomainId(model.GetDomainId());
    getTokenRequeset.SetEndpoint(model.GetEndpoint());
    getTokenRequeset.SetEnvAddress(model.GetEnvAddress());
    getTokenRequeset.SetNeedRetry(false);
    KeyStoneClient keystoneClient;
    std::shared_ptr<GetTokenResponse> getTokenResponse = keystoneClient.GetToken(getTokenRequeset);
    if (getTokenResponse == nullptr) {
        ERRLOG("Failed to get token by keystone api.");
        return false;
    }
    if (getTokenResponse->GetStatusCode() != static_cast<uint32_t>(Module::SC_CREATED)) {
        ERRLOG("Failed to get token by keystone api, status: %d, resp body: %s.", getTokenResponse->GetStatusCode(),
            getTokenResponse->GetBody().c_str());
        return false;
    }
    if (!getTokenResponse->Serial()) {
        ERRLOG("Failed to serial token requeset.");
        return false;
    }
    tokenValue = AddToken(model, getTokenResponse);
    if (tokenValue.empty()) {
        ERRLOG("Failed to save token.");
        return false;
    }
    return true;
}

/**
 * @brief 获取request对应的EndPoint
 *
 * @param model
 * @param tokenInfo
 * @param endPoint
 * @return true
 * @return false
 */
bool OpenStackTokenMgr::ParseEndpoint(ModelBase &model, const TokenInfo &tokenInfo, std::string &endPoint)
{
    std::vector<std::string> novaApiNames = {"nova"};
    std::vector<std::string> cinderApiNames = {"cinderv3", "cinderv2"};
    std::vector<std::string> neutronApiName = {"neutron"};
    std::map<ApiType, std::vector<std::string>> apiMap = {{ApiType::NOVA, novaApiNames},
        {ApiType::CINDER, cinderApiNames}, {ApiType::NEUTRON, neutronApiName}};
    auto it = apiMap.find(model.GetApiType());
    if (it == apiMap.end()) {
        DBGLOG("Not need to parse endpoint.");
        return true;
    }
    std::vector<std::string> apiNames = it->second;
    TokenDetail tokenDetail;
    if (!Module::JsonHelper::JsonStringToStruct(tokenInfo.m_extendInfo, tokenDetail)) {
        ERRLOG("Failed to trans string to tokenDetail.");
        return false;
    }
    std::vector<Catalog> catalogs = tokenDetail.m_token.m_catalog;
    std::vector<Endpoint> endpoints;
    for (const auto &apiName : it->second) {
        for (const auto &catalog : catalogs) {
            if (apiName != catalog.m_name) {
                continue;
            }
            endpoints = catalog.m_endpoints;
            break;
        }
        if (!endpoints.empty()) {
            break;
        }
    }
    for (const auto &endpoint : endpoints) {
        if (endpoint.m_interface == "public") {
            endPoint = endpoint.m_url;
            return true;
        }
    }
    ERRLOG("Failed to get endpoint.");
    return false;
}
OPENSTACK_PLUGIN_NAMESPACE_END
