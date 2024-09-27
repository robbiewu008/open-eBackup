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
#include "BaseTokenMgr.h"
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "log/Log.h"
#include "common/utils/Utils.h"
#ifdef WIN32
#include <windows.h>
#else
#endif

using namespace std;
namespace {
const int32_t CHECK_TOKEN_SLEEP_TIME = 60 * 60; // s
const int32_t EARLY_EXPIRATION_TIME = 30;   // s
const std::string MODULE_NAME = "BaseTokenMgr";


const std::string KEY_TOKEN_PREFIX_ADMIN_PROJECT = "_admin_project_";
const int32_t DATE_SPLITE_LENGTH = 2;
}

VIRT_PLUGIN_NAMESPACE_BEGIN

static std::once_flag startCheckTokenThread;

void BaseTokenMgr::CheckToken()
{
    while (true) {
        Utils::SleepSeconds(CHECK_TOKEN_SLEEP_TIME);
        std::lock_guard<std::mutex> locker(m_tokenMutex);
        std::map<std::string, TokenInfo>::iterator it = m_tokenMap.begin();
        while (it != m_tokenMap.end()) {
            std::string expiresDate = it->second.m_expiresDate; // 2022-07-06T07:04:08.361000Z
            if (TokenIsExpired(expiresDate)) {
                m_tokenMap.erase(it++);
            } else {
                ++it;
            }
        }
    }
}

bool BaseTokenMgr::TokenIsExpired(std::string &date)
{
    int64_t nowTimeStamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string timeStamp;
    std::vector<std::string> dateSpliteStrs;
    (void)boost::split(dateSpliteStrs, date, boost::is_any_of("."));
    if (dateSpliteStrs.size() != DATE_SPLITE_LENGTH) {
        return false;
    }
    timeStamp = dateSpliteStrs[0];
    if (!DateTransToStamp(date, timeStamp)) {
        ERRLOG("Failed to trans date to time stamp.");
        return false;
    }
    int64_t expiresTimeStamp = std::stoi(timeStamp);
    int timeStampDiff = GetTimeDiffWithGMT();
    if ((expiresTimeStamp + timeStampDiff - EARLY_EXPIRATION_TIME) < nowTimeStamp) { // 提前30秒过期
        return true;
    }
    return false;
}

bool BaseTokenMgr::GetToken(ModelBase &model, std::string &tokenValue, std::string &endPoint)
{
    if (!model.IsNeedNewToken()) {
        if (GetTokenFromMap(model, tokenValue, endPoint, true)) {
            INFOLOG("Success to get buffer token.");
            return true;
        }
    }
    if (!ReacquireToken(model, tokenValue)) { // 重新获取token
        ERRLOG("Failed to reacquire token.");
        return false;
    }
    if (!GetTokenFromMap(model, tokenValue, endPoint, false)) {
        ERRLOG("Failed to get token.");
        return false;
    }
    INFOLOG("Success to get new token.");
    return true;
}

std::string BaseTokenMgr::AddToken(ModelBase &model, std::shared_ptr<GetTokenResponse> getTokenResponse)
{
    std::string tokenValue = "";
    if (getTokenResponse == nullptr) {
        ERRLOG("The token response is null.");
        return tokenValue;
    }
    TokenInfo tokenInfoTmp;
    auto headers = getTokenResponse->GetHeaders();
    auto it_head = headers.find("X-Subject-Token");
    if (it_head == headers.end()) {
        ERRLOG("Not found head key X-Subject-Token");
        return tokenValue;
    }
    tokenInfoTmp.m_token = *(it_head->second.begin());
    tokenInfoTmp.m_extendInfo = getTokenResponse->GetBody();
    tokenInfoTmp.m_expiresDate = getTokenResponse->GetTokenDetail().m_token.m_expiresAt;
    std::string key = GetTokenKey(model);
    m_tokenMap[key] = tokenInfoTmp;
    tokenValue = tokenInfoTmp.m_token;
    return tokenValue;
}

bool BaseTokenMgr::GetTokenFromMap(ModelBase &model, std::string &tokenValue, std::string &endPoint,
    const bool &checkExpireFlag)
{
    std::lock_guard<std::mutex> locker(m_tokenMutex);
    std::string key = GetTokenKey(model);
    auto it = m_tokenMap.find(key);
    if (it == m_tokenMap.end()) {
        DBGLOG("Not found token, the key:%s", key.c_str());
        return false;
    }
    if (checkExpireFlag) {    // 检验是否过期
        std::string expiresDate = it->second.m_expiresDate;
        if (TokenIsExpired(expiresDate)) {
            ERRLOG("Token expired.");
            return false;
        }
    }
    TokenInfo &tokenInfo = it->second;
    tokenValue = tokenInfo.m_token;
    if (!ParseEndpoint(model, tokenInfo, endPoint)) {
        ERRLOG("Failed to parse endpoint.");
        return false;
    }
    return true;
}

bool BaseTokenMgr::DateTransToStamp(const std::string &date, std::string &timeStamp) const
{
    tm timep;
    timep.tm_isdst = -1;
    if (date.empty()) {
        ERRLOG("Date time Param is empty.");
        return false;
    }
#ifndef WIN32
    if (strptime(date.c_str(), "%Y-%m-%dT%H:%M:%S", &timep) == nullptr) {
        ERRLOG("Transform date to timestamp failed.");
        return false;
    }
#else
    // ->:windows实现后续补充
#endif
    time_t unixStamp = mktime(&timep);
    if (unixStamp == -1) {
        ERRLOG("Date time mktime failed.");
        return false;
    }
    timeStamp = std::to_string(unixStamp);
    INFOLOG("timeStamp.%s", timeStamp.c_str());
    return true;
}

void BaseTokenMgr::DeleteToken(ModelBase& model)
{
    std::lock_guard<std::mutex> locker(m_tokenMutex);
    std::string tokenKey = GetTokenKey(model);
    auto it = m_tokenMap.find(tokenKey);
    if (it != m_tokenMap.end()) {
        m_tokenMap.erase(it);
        INFOLOG("Delete token success");
    }
}

VIRT_PLUGIN_NAMESPACE_END