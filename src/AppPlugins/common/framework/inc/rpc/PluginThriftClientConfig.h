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
#ifndef PLUGIN_THRIFT_CLIENT_CONFIG
#define PLUGIN_THRIFT_CLIENT_CONFIG

#include <string>
#include "PluginTypes.h"
#ifdef WIN32
#include "define/Defines.h"
#endif

namespace startup {
#ifdef WIN32
class AGENT_API PluginThriftClientConfig {
#else
class PluginThriftClientConfig {
#endif
public:
    static PluginThriftClientConfig& GetInstance();
    void Configure(std::string ip, uint32_t port);
    std::string GetServerIP();
    uint32_t GetServerPort();

private:
    static PluginThriftClientConfig m_instance;
    PluginThriftClientConfig();
    ~PluginThriftClientConfig();
    std::string m_serverIP;
    uint32_t m_serverPort { 0 };
};
}

#endif