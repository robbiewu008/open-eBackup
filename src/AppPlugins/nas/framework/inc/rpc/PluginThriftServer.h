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
#ifndef PLUGIN_THRIFT_SERVER_H
#define PLUGIN_THRIFT_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include "PluginTypes.h"
#include "thriftservice/IThriftService.h"

class PluginThriftServer {
public:
    static PluginThriftServer& GetInstance();
    void Configure(std::string ip, uint32_t port);
    bool Start();
    bool Stop();

private:
    PluginThriftServer();
    ~PluginThriftServer();
    bool Init();
    std::string m_ip;
    uint32_t m_port { 0 };
    std::vector<std::string> m_services;
    std::shared_ptr<thriftservice::IThriftService> m_thriftService { nullptr };
    std::shared_ptr<thriftservice::IThriftServer> m_thriftServer { nullptr };
};

#endif // _PLUGIN_THRIFT_SERVER_H_