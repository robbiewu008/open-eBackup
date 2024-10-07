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
#include "security/cmd/CmdExecutor.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdPolicy.h"
#include "log/Log.h"

namespace Module {

bool RegisterWhitelist(
    const std::unordered_set<std::string>& cmdWhitelist, const std::unordered_set<std::string>& pathWhitelist)
{
    try {
        return CmdPolicy::RegisterWhitelist(cmdWhitelist, pathWhitelist);
    } catch (std::exception& e) {
        ERRLOG("Failed to register whitelist: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    return false;
}

int RunCommand(const std::string& cmdName, const std::vector<std::string>& cmdVec, std::vector<std::string>& result,
    const CmdRunUser& cmdUser)
{
    try {
        CmdBuilder builder = CmdBuilder(cmdName, cmdVec);
        if (!builder.Init()) {
            ERRLOG("Failed to build command: %s", WIPE_SENSITIVE(cmdName).c_str());
            return -1;
        }

        if (!cmdUser.username.empty()) {
            builder.SetRunUser(cmdUser);
        }

        return builder.Run(result);
    } catch (std::exception& e) {
        ERRLOG("Failed to register whitelist: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    return -1;
}

int RunCommand(const std::string& cmdName, const std::vector<CmdParam>& cmdVec, std::vector<std::string>& result,
    const CmdRunUser& cmdUser)
{
    try {
        CmdBuilder builder = CmdBuilder(cmdName, cmdVec);
        if (!builder.Init()) {
            ERRLOG("Failed to build command: %s", WIPE_SENSITIVE(cmdName).c_str());
            return -1;
        }

        if (!cmdUser.username.empty()) {
            builder.SetRunUser(cmdUser);
        }

        return builder.Run(result);
    } catch (std::exception& e) {
        ERRLOG("Failed to register whitelist: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    return -1;
}

int RunCommand(const std::string& cmdName, const std::vector<CmdParam>& cmdVec, std::vector<std::string>& result,
    const std::unordered_set<std::string>& pathWhitelist, const CmdRunUser& cmdUser)
{
    try {
        CmdBuilder builder = CmdBuilder(cmdName, cmdVec);
        if (!pathWhitelist.empty()) {
            builder.SetPathWhitelist(pathWhitelist);
        }

        if (!builder.Init()) {
            ERRLOG("Failed to build command: %s", WIPE_SENSITIVE(cmdName).c_str());
            return -1;
        }

        if (!cmdUser.username.empty()) {
            builder.SetRunUser(cmdUser);
        }

        return builder.Run(result);
    } catch (std::exception& e) {
        ERRLOG("Failed to register whitelist: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    return -1;
}

int RunCommand(CmdBuilder& builder, std::vector<std::string>& result)
{
    try {
        if (!builder.Init()) {
            ERRLOG("Failed to build command: %s", WIPE_SENSITIVE(builder.GetPrintCmdString()).c_str());
            return -1;
        }
        return builder.Run(result);
    } catch (std::exception& e) {
        ERRLOG("Failed to register whitelist: %s", WIPE_SENSITIVE(e.what()).c_str());
    }

    return -1;
}

}
