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
#include "NutanixSession.h"
#include "common/Constants.h"
namespace NutanixPlugin {
std::shared_ptr<NutanixSessionCacheManage> NutanixSessionCacheManage::m_instancePtr = nullptr;
std::mutex NutanixSessionCacheManage::m_cacheMutex;
namespace {
    const int32_t SESSION_UPDATE_TIMEOUT_SECOND = 3;
    const int32_t IP_INDEX_IN_KEY = 0;
    const int32_t PORT_INDEX_IN_KEY = 1;
    const int32_t USER_INDEX_IN_KEY = 2;
}

SessionCacheRetvalue NutanixSessionCacheManage::Get(const IP_PORT_USER_KEY &info, NutanixSession &seesion)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::map< IP_PORT_USER_KEY, NutanixSessionCache >::iterator itr = m_sessionCache.find(info);
    if (itr != m_sessionCache.end()) {
        // 如果session是无效的，且在更新中;
        uint64_t tmpTime = Module::CTime::GetTimeSec();
        // 距上次更新超过3s无效就再次发起更新;
        if (tmpTime > (itr->second.m_lasteUpdateTime + SESSION_UPDATE_TIMEOUT_SECOND)) {
            itr->second.m_lasteUpdateTime = tmpTime;
            WARNLOG("update timeout the session %s:%s, user:%s", std::get<IP_INDEX_IN_KEY>(info).c_str(),
                std::get<PORT_INDEX_IN_KEY>(info).c_str(), std::get<USER_INDEX_IN_KEY>(info).c_str());
            return SessionCacheRetvalue::SESSION_IS_INVALID_AND_NEED_UPDATE;
        }
        // 考虑到夏时令跳变;
        if (tmpTime < itr->second.m_lasteUpdateTime) {
            itr->second.m_lasteUpdateTime = tmpTime;
            WARNLOG("DST change the session %s:%s, user:%s", std::get<IP_INDEX_IN_KEY>(info).c_str(),
                std::get<PORT_INDEX_IN_KEY>(info).c_str(), std::get<USER_INDEX_IN_KEY>(info).c_str());
            return SessionCacheRetvalue::SESSION_IS_INVALID_AND_NEED_UPDATE;
        }
        if (itr->second.m_session != nullptr) {
            // 如果session是有效的就直接返回;
            seesion = *(itr->second.m_session);
            return SessionCacheRetvalue::SESSION_IS_VALID;
        }
        WARNLOG("updating the session %s:%s, user:%s", std::get<IP_INDEX_IN_KEY>(info).c_str(),
            std::get<PORT_INDEX_IN_KEY>(info).c_str(), std::get<USER_INDEX_IN_KEY>(info).c_str());
        return SessionCacheRetvalue::SESSION_IS_INVALID_AND_NONEED_UPDATE;
    }  else {
        // 第一次无session场景;
        NutanixSessionCache tmpCache(nullptr);
        m_sessionCache[info] = tmpCache;
        return SessionCacheRetvalue::SESSION_IS_INVALID_AND_NEED_UPDATE;
    }
}

void NutanixSessionCacheManage::Update(const IP_PORT_USER_KEY &info, std::shared_ptr<NutanixSession> session)
{
    int32_t count = 0;
    {
        std::lock_guard<std::mutex> lock(m_lock);
        // 因为session都是拷贝值走的，所以不会内存被释放;
        NutanixSessionCache tmpCache(session);
        m_sessionCache[info] = tmpCache;
        count = m_sessionCache.size();
    }
    WARNLOG("update the session success %s:%s, user:%s, count:%d", std::get<IP_INDEX_IN_KEY>(info).c_str(),
        std::get<PORT_INDEX_IN_KEY>(info).c_str(), std::get<USER_INDEX_IN_KEY>(info).c_str(), count);
}

void NutanixSessionCacheManage::Clean(const IP_PORT_USER_KEY &info, std::shared_ptr<NutanixSession> session)
{
    int32_t count = 0;
    {
        std::lock_guard<std::mutex> lock(m_lock);
        // 因为session都是拷贝值走的，所以不会内存被释放;
        std::map< IP_PORT_USER_KEY, NutanixSessionCache >::iterator itr = m_sessionCache.find(info);
        if (itr != m_sessionCache.end() && itr->second.m_session != nullptr && *(itr->second.m_session) == *session) {
            m_sessionCache.erase(itr);
        }
        count = m_sessionCache.size();
    }
    WARNLOG("erase the session success %s:%s, user:%s, count:%d", std::get<IP_INDEX_IN_KEY>(info).c_str(),
        std::get<PORT_INDEX_IN_KEY>(info).c_str(), std::get<USER_INDEX_IN_KEY>(info).c_str(), count);
}
};