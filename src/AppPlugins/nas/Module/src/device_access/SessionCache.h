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
#include "define/Types.h"
#include "common/CleanMemPwd.h"

namespace Module {
// Session without mutex
    struct SessionInfo {
        std::string device_id{};
        std::string token{};
        std::string cookie{};

        SessionInfo() {}

        ~SessionInfo() {
            Module::CleanMemoryPwd(token);
        }

        SessionInfo(const SessionInfo &obj) {
            this->token = obj.token;
            this->cookie = obj.cookie;
            this->device_id = obj.device_id;
        }

        SessionInfo &operator=(const SessionInfo &obj) {
            if (this == &obj)
                return *this;
            this->token = obj.token;
            this->cookie = obj.cookie;
            this->device_id = obj.device_id;
            return *this;
        }
    };

// Session with mutex
    struct Session {
        std::string deviceId;
        std::string token;
        std::string cookie;
        std::mutex sessionMutex;

        Session() {
            deviceId = "";
            token = "";
            cookie = "";
        }

        Session(std::string _token, std::string _deviceId, std::string _cookie) {
            this->token = _token;
            this->deviceId = _deviceId;
            this->cookie = _cookie;
        }

        Session(const Session &obj) {
            this->token = obj.token;
            this->cookie = obj.cookie;
            this->deviceId = obj.deviceId;
        }

        Session &operator=(const Session &obj) {
            if (this == &obj)
                return *this;
            this->token = obj.token;
            this->cookie = obj.cookie;
            this->deviceId = obj.deviceId;
            return *this;
        }
    };

    class SessionCache {

    private:
        std::string deviceType;                                            // DORAOD or OCEANSTOR OR FS OR NETAPP...
        std::mutex cacheMutex;                                             // Mutex to protect cache
        std::map<std::tuple<std::string, std::string, std::string>,
                std::tuple<std::shared_ptr<Session>, int>> m_sessionCache;     // Session Cache

    public:
        explicit SessionCache(std::string deviceType) {
            this->deviceType = deviceType;
        }

        ~SessionCache() {}

        std::shared_ptr<Session> CreateSession(std::string deviceIp, std::string deviceUserName,
                                               std::string devicePort, std::function<SessionInfo()> pLogin);

        bool DeleteSession(std::string deviceIp, std::string deviceUserName,
                           std::string devicePort, std::function<mp_int32(SessionInfo)> pLogout);

        /* Warning - Get session increase refCnt
         *           Must call delete session to decrease refCnt
         */
        std::shared_ptr<Session> GetSession(std::string deviceIp, std::string deviceUserName,
                                            std::string devicePort);
    };
}

#endif