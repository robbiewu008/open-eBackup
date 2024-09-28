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
#include "log/Log.h"
#include "EnvVarManager.h"

using namespace std;
namespace Module {

namespace {
const string ENV_KEY_DATA_BACKUP_AGENT_HOME = "DATA_BACKUP_AGENT_HOME";
}

string EnvVarManager::GetEnv(const std::string& envKey)
{
    string envVal = (std::getenv(envKey.c_str()) == nullptr ? "" : std::getenv(envKey.c_str()));
    if (!envVal.empty()) {
        return envVal;
    }
    ERRLOG("GetEnv failed, The environment variable: %s is empty!", envKey.c_str());
    if (envKey == ENV_KEY_DATA_BACKUP_AGENT_HOME) {
#ifdef WIN32
        envVal = "C:";
#else
        envVal = "/opt";
#endif
    }
    return envVal;
}

string EnvVarManager::GetAgentHomePath()
{
    if (m_agentHomePath.empty()) {
        m_agentHomePath = GetEnv(ENV_KEY_DATA_BACKUP_AGENT_HOME);
    }
    return m_agentHomePath;
}

string EnvVarManager::GetPluginInstallPath()
{
    if (m_pluginInstallPath.empty()) {
        m_pluginInstallPath = GetEnv(ENV_KEY_DATA_BACKUP_AGENT_HOME) + PATH_SEPARATOR + "DataBackup";
    }
    return m_pluginInstallPath;
}

#ifdef WIN32
string EnvVarManager::GetAgentWin7zPath()
{
    if (m_agentWin7zPath.empty()) {
        m_agentWin7zPath = 
            GetEnv(ENV_KEY_DATA_BACKUP_AGENT_HOME) + R"(\DataBackup\ProtectClient\ProtectClient-E\bin\7z.exe)";
    }
    return m_agentWin7zPath;
}
#endif
}
