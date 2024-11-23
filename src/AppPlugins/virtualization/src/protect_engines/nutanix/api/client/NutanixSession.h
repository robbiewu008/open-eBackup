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
#ifndef VIRTUALIZATION_PLUGIN_NUTANIXSESSION_H
#define VIRTUALIZATION_PLUGIN_NUTANIXSESSION_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <tuple>
#include <map>
#include <string>
#include <mutex>
#include "define/Types.h"
#include "json/json.h"
#include "log/Log.h"
#include "common/CleanMemPwd.h"
#include "curl_http/HttpClientInterface.h"
#include "common/CTime.h"

namespace NutanixPlugin {
    using IP_PORT_USER_KEY = std::tuple<std::string, std::string, std::string>;
    enum class SessionCacheRetvalue {
        SESSION_IS_VALID,
        SESSION_IS_INVALID_AND_NEED_UPDATE,
        SESSION_IS_INVALID_AND_NONEED_UPDATE,
    };
class NutanixSession {
public:
    ~NutanixSession()
    {
        Module::CleanMemoryPwd(m_cookie);
    }
    NutanixSession() noexcept
    {
        m_cookie = "";
    }
    explicit NutanixSession(std::set<std::string> cookies)
    {
        for (const std::string &ret : cookies) {
            m_cookie.append(ret);
            m_cookie.append(";");
        }
    }

    explicit NutanixSession(std::string cookie)
        : m_cookie(cookie)
    {
    }
    NutanixSession(const NutanixSession &obj)
    {
        this->m_cookie = obj.m_cookie;
    }
    NutanixSession &operator=(const NutanixSession &obj)
    {
        if (this == &obj) {
            return *this;
        }
        this->m_cookie = obj.m_cookie;
        return *this;
    }
    bool operator==(const NutanixSession &obj)
    {
        return m_cookie == obj.m_cookie;
    }

    std::string m_cookie;
};
class NutanixSessionCache {
public:
    NutanixSessionCache()
    {
        m_lasteUpdateTime = Module::CTime::GetTimeSec();
        m_session = nullptr;
    }

    explicit NutanixSessionCache(std::shared_ptr<NutanixSession> session)
    {
        m_lasteUpdateTime = Module::CTime::GetTimeSec();
        m_session = session;
    }

    ~NutanixSessionCache() {}

    NutanixSessionCache(const NutanixSessionCache &obj)
    {
        m_lasteUpdateTime = obj.m_lasteUpdateTime;
        m_session = obj.m_session;
    }

    NutanixSessionCache &operator=(const NutanixSessionCache &obj)
    {
        if (this == &obj) {
            return *this;
        }
        m_lasteUpdateTime = obj.m_lasteUpdateTime;
        m_session = obj.m_session;
        return *this;
    }

    uint64_t m_lasteUpdateTime;
    std::shared_ptr<NutanixSession> m_session;
};
class NutanixSessionCacheManage {
public:
    NutanixSessionCacheManage() {};
    ~NutanixSessionCacheManage() {}
    static std::shared_ptr<NutanixSessionCacheManage> GetInstance()
    {
        if (m_instancePtr == nullptr) {
            std::lock_guard<std::mutex> lk(m_cacheMutex);
            if (m_instancePtr == nullptr) {
                m_instancePtr = std::make_shared<NutanixSessionCacheManage>();
            }
        }
        return m_instancePtr;
    }

    void Update(const IP_PORT_USER_KEY &info, std::shared_ptr<NutanixSession> session);
    void Clean(const IP_PORT_USER_KEY &info, std::shared_ptr<NutanixSession> session);

    SessionCacheRetvalue Get(const IP_PORT_USER_KEY &info, NutanixSession &seesion);

private:
    NutanixSessionCacheManage(NutanixSessionCacheManage&) = delete;
    NutanixSessionCacheManage& operator=(const NutanixSessionCacheManage&) = delete;

    static std::shared_ptr<NutanixSessionCacheManage> m_instancePtr;
    static std::mutex m_cacheMutex;

    std::mutex m_lock;
    // IP + Port + userID 作为key的三要素;
    std::map< IP_PORT_USER_KEY, NutanixSessionCache > m_sessionCache;
};
};
#endif
