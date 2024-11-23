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
#include "CNwareSession.h"
#include "common/Constants.h"

namespace {
const double CNWARE_SESSION_LIMIT = 1680;
};

namespace CNwarePlugin {
std::shared_ptr<CNwareSessionCache> CNwareSessionCache::m_instancePtr = nullptr;
std::mutex CNwareSessionCache::m_cacheMutex;

std::shared_ptr<CNwareSession> CNwareSessionCache::GetCNwareSession(
    const std::tuple<std::string, std::string>& cnwareInfo)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    auto itr = m_sessionCache.find(cnwareInfo);
    if (itr != m_sessionCache.end()) {
        return itr->second;
    }
    return nullptr;
}

void CNwareSessionCache::AddCNwareSession(std::shared_ptr<CNwareSession> session)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_sessionCache[std::make_tuple(session->m_ip, session->m_userId)] = session;
}

bool CNwareSessionCache::IsSessionRemain()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    WARNLOG("IsSessionRemain IsSessionRemain IsSessionRemain!");
    return (!m_sessionCache.empty());
}

std::shared_ptr<CNwareSession> CNwareSessionCache::GetLastCNwareSession()
{
    INFOLOG("Get last CNwareSession");
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_sessionCache.empty()) {
        auto iter = m_sessionCache.rbegin();
        auto targetPtr = iter->second;
        DBGLOG("Get last CNwareSession success!");
        return targetPtr;
    }
    return nullptr;
}

void CNwareSessionCache::EraseSession(const std::tuple<std::string, std::string>& key)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_sessionCache.erase(key);
    return;
}
};