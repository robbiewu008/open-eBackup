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
#include "device_access/SessionCache.h"

namespace Module {
    std::shared_ptr<Session> SessionCache::CreateSession(std::string deviceIp, std::string deviceUserName,
                                                         std::string devicePort, std::function<SessionInfo()> pLogin)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        std::tuple <std::string, std::string, std::string> key =
            std::make_tuple(deviceIp, deviceUserName, devicePort);
        auto itr = m_sessionCache.find(key);
        if (itr != m_sessionCache.end()) {
            std::get<1>(itr->second)++;
            return std::get<0>(itr->second);
        }

        SessionInfo m_sessionInfo = pLogin();
        if (m_sessionInfo.token.empty() || m_sessionInfo.cookie.empty() ||
            m_sessionInfo.device_id.empty()) {
            HCP_Log(ERR, deviceType) << "Some of received session fields are empty" << HCPENDLOG;
            return nullptr;
        }
        std::shared_ptr<Session> sessionPtr = std::make_shared<Session>(m_sessionInfo.token,
                                                                        m_sessionInfo.device_id,
                                                                        m_sessionInfo.cookie);
        m_sessionCache.emplace(key, std::make_tuple(sessionPtr, 1));
        return sessionPtr;
    }

    bool SessionCache::DeleteSession(std::string deviceIp, std::string deviceUserName,
                                     std::string devicePort, std::function<mp_int32(SessionInfo)> pLogout)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        std::tuple <std::string, std::string, std::string> key =
            std::make_tuple(deviceIp, deviceUserName, devicePort);
        auto itr = m_sessionCache.find(key);
        if (itr == m_sessionCache.end()) {
            HCP_Log(INFO, deviceType) << "No session to delete from map" << HCPENDLOG;
            return true;
        }

        std::get<1>(itr->second)--;
        if (std::get<1>(itr->second) == 0) {
            SessionInfo m_sessionInfo;
            std::shared_ptr<Session> sessionPtr = std::get<0>(itr->second);
            m_sessionInfo.token = sessionPtr->token;
            m_sessionInfo.cookie = sessionPtr->cookie;
            m_sessionInfo.device_id = sessionPtr->deviceId;

            mp_int32 ret = pLogout(m_sessionInfo);
            if (ret != SUCCESS) {
                HCP_Log(ERR, deviceType) << "Logout not success, But entry is deleted "
                                         << ret << HCPENDLOG;
            }
            m_sessionCache.erase(itr);
        }
        return true;
    }

/* Warning - Get session increase refCnt
     *           Must call delete session to decrease refCnt
     */
    std::shared_ptr<Session> SessionCache::GetSession(std::string deviceIp, std::string deviceUserName,
                                                      std::string devicePort)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        std::tuple <std::string, std::string, std::string> key =
            std::make_tuple(deviceIp, deviceUserName, devicePort);
        auto itr = m_sessionCache.find(key);
        if (itr != m_sessionCache.end()) {
            std::get<1>(itr->second)++;
            return std::get<0>(itr->second);
        }
        return nullptr;
    }
}
