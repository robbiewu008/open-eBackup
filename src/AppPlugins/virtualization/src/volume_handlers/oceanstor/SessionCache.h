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
#ifndef SESSION_CACHE_H
#define SESSION_CACHE_H

#include <tuple>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <utility>
#include <functional>
#include "log/Log.h"
#include "common/CleanMemPwd.h"
#include "common/Macros.h"

namespace VirtPlugin {
// StorageSessionInfo without mutex
struct StorageSessionInfo {
    std::string deviceId{};
    std::string token{};
    std::string cookie{};
    std::string ip{};
    std::mutex sessionMutex{};

    StorageSessionInfo() noexcept
    {
    }
    ~StorageSessionInfo()
    {
        Module::CleanMemoryPwd(token);
    }

    StorageSessionInfo(const StorageSessionInfo &obj) noexcept
    {
        this->token = obj.token;
        this->cookie = obj.cookie;
        this->deviceId = obj.deviceId;
        this->ip = obj.ip;
    }

    StorageSessionInfo &operator=(const StorageSessionInfo &obj)
    {
        if (this == &obj) {
            return *this;
        }
        this->token = obj.token;
        this->cookie = obj.cookie;
        this->deviceId = obj.deviceId;
        this->ip = obj.ip;
        return *this;
    }
};

// StorageSession with mutex
struct StorageSession {
    std::string deviceId;
    std::string token;
    std::string cookie;
    std::mutex sessionMutex;
    StorageSession()
    {
        deviceId = "";
        token = "";
        cookie = "";
    }

    StorageSession(std::string token, std::string deviceId, std::string cookie)
        : token(token), deviceId(deviceId), cookie(cookie)
    {}

    StorageSession(const StorageSession &obj)
    {
        this->token = obj.token;
        this->cookie = obj.cookie;
        this->deviceId = obj.deviceId;
    }

    StorageSession &operator=(const StorageSession &obj)
    {
        if (this == &obj) {
            return *this;
        }
        this->token = obj.token;
        this->cookie = obj.cookie;
        this->deviceId = obj.deviceId;
        return *this;
    }
};

class SessionCache {
public:
    explicit SessionCache(std::string deviceType) : deviceType(deviceType)
    {}
    ~SessionCache()
    {}
    bool CreateSession(std::string deviceIp, std::string deviceUserName, std::string &errorDes,
        std::string devicePort, std::function<StorageSessionInfo(std::string&, const std::string&)> pLogin);
    bool DeleteSession(const std::string deviceIp, const std::string deviceUserName, const std::string devicePort,
                       std::function<int32_t(StorageSessionInfo)> pLogout);
    bool CheckSession(std::string deviceIp, std::string deviceUserName, std::string devicePort);
    void RefreshSession(StorageSessionInfo sessionInfo, std::string deviceUserName, std::string devicePort);
    /* Warning - Get session increase refCnt
     *           Must call delete session to decrease refCnt
     */
    std::shared_ptr<StorageSession> GetSession(std::string deviceIp, std::string deviceUserName,
                                               std::string devicePort);

private:
    std::string deviceType;  // DORAOD or OCEANSTOR OR FS OR NETAPP...
    std::mutex cacheMutex;   // Mutex to protect cache
    std::map<std::tuple<std::string, std::string, std::string>, std::tuple<std::shared_ptr<StorageSession>, int> >
        m_sessionCache;
};
}

#endif