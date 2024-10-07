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
#ifndef MODULE_ENV_VAR_MANAGER_H
#define MODULE_ENV_VAR_MANAGER_H

#include <cstdlib>
#include <string>
#include "define/Defines.h"

namespace Module {
class AGENT_API EnvVarManager
{
public:
    static EnvVarManager* GetInstance()
    {
        static EnvVarManager instance;
        return &instance;
    }
    ~EnvVarManager() {};
    std::string GetEnv(const std::string& envKey);
    std::string GetAgentHomePath();
    std::string GetPluginInstallPath();
#ifdef WIN32
    std::string GetAgentWin7zPath();
#endif
private:
    EnvVarManager() {};

    std::string m_agentHomePath {};
    std::string m_pluginInstallPath {};
    std::string m_agentWin7zPath {};
};
}
#endif