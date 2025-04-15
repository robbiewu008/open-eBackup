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
#ifndef VIRTUALIZATION_PLUGIN_CNWARESESSION_H
#define VIRTUALIZATION_PLUGIN_CNWARESESSION_H

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

namespace CNwarePlugin {

// StorageSession with mutex
class CNwareSession {
public:
    ~CNwareSession()
    {
        Module::CleanMemoryPwd(m_cookie);
    }
    std::string m_ip;
    std::string m_userId;
    std::string m_cookie;
    std::string m_port;
    std::mutex sessionMutex;
    CNwareSession() noexcept
    {
        m_userId = "";
        m_ip = "";
        m_cookie = "";
    }
    CNwareSession(std::string ip, std::string port, std::string userId, std::string cookie)
        : m_ip(ip), m_userId(userId), m_cookie(cookie), m_port(port)
    {
    }
    CNwareSession(const CNwareSession &obj)
    {
        this->m_ip = obj.m_ip;
        this->m_cookie = obj.m_cookie;
        this->m_userId = obj.m_userId;
        this->m_port = obj.m_port;
    }
    CNwareSession &operator=(const CNwareSession &obj)
    {
        if (this == &obj) {
            return *this;
        }
        this->m_ip = obj.m_ip;
        this->m_cookie = obj.m_cookie;
        this->m_userId = obj.m_userId;
        this->m_port = obj.m_port;
        return *this;
    }
};

class CNwareSessionCache {
public:
    ~CNwareSessionCache() {}
    static std::shared_ptr<CNwareSessionCache> GetInstance()
    {
        if (m_instancePtr == nullptr) {
            std::lock_guard<std::mutex> lk(m_cacheMutex);
            m_instancePtr = std::shared_ptr<CNwareSessionCache>(new CNwareSessionCache);
        }
        return m_instancePtr;
    }

    void AddCNwareSession(std::shared_ptr<CNwareSession> session);
    std::shared_ptr<CNwareSession> GetCNwareSession(const std::tuple<std::string, std::string>& cnwareInfo);
    std::shared_ptr<CNwareSession> GetLastCNwareSession();
    bool IsSessionRemain();
    void EraseSession(const std::tuple<std::string, std::string>& key);
    void IncreaseRegistreCnt();
    void DecreaseRegistreCnt();
    bool NeedRelease();

private:
    CNwareSessionCache() {}
    CNwareSessionCache(CNwareSessionCache&)=delete;
    CNwareSessionCache& operator=(const CNwareSessionCache&)=delete;

private:
    static std::shared_ptr<CNwareSessionCache> m_instancePtr;
    static std::mutex m_cacheMutex;
    int32_t m_useSessionNums {0};
    std::map<std::tuple<std::string, std::string>, std::shared_ptr<CNwareSession> >
        m_sessionCache = {};
};
};
#endif // VIRTUALIZATION_PLUGIN_CNWARESESSION_H
