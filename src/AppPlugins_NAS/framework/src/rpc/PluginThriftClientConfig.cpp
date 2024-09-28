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
#include "PluginThriftClientConfig.h"

using namespace startup;

PluginThriftClientConfig PluginThriftClientConfig::m_instance;

PluginThriftClientConfig::PluginThriftClientConfig()
{}

PluginThriftClientConfig::~PluginThriftClientConfig()
{}

PluginThriftClientConfig& PluginThriftClientConfig::GetInstance()
{
    return m_instance;
}

void PluginThriftClientConfig::Configure(std::string ip, uint32_t port)
{
    m_serverIP = ip;
    m_serverPort = port;
}

std::string PluginThriftClientConfig::GetServerIP()
{
    return m_serverIP;
}

uint32_t PluginThriftClientConfig::GetServerPort()
{
    return m_serverPort;
}
