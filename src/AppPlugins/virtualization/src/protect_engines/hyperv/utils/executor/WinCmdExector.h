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
#ifndef __WINCMD_EXECALLER_H__
#define __WINCMD_EXECALLER_H__

#ifdef WIN32
#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <WinBase.h>
#include "common/JsonHelper.h"

namespace HyperVPlugin {
namespace Utils {

typedef struct Param {
    Json::Value param;
    std::string cmdType;
    std::string scriptDir;
    std::string scriptName;
    std::string userName;
    std::string password;
    std::string requestId;
    DWORD timeout = INFINITE;
} Param;

enum class CMD_RUNNING_TARGET {
    TYPE_UNKNOWN = 0,
    TYPE_SCVMM = 1,
    TYPE_FO_CLUSTER,
    TYPE_HOST
};

class WinCmdExector {
public:
    ~WinCmdExector()
    {
        CleanMemorySenInfo(m_param.password);
    }
    static WinCmdExector &GetInstance()
    {
        static WinCmdExector execaller;
        return execaller;
    }

public:
    int Execute(const Param &cmdParam, Json::Value &result);

    /*
     * Command running target. If not setting or setting to empty,
     * means the command is going to run in localhost.
     */
    void SetTargetHost(const std::string &host)
    {
        m_targetHost = host;
    }
    std::string GetTargetHost()
    {
        return m_targetHost;
    }

    /* Command running target type. If not setting, means it is a TYPE_HOST command. */
    void SetTargetType(const CMD_RUNNING_TARGET &type)
    {
        m_targetType = type;
    }
    CMD_RUNNING_TARGET GetTargetType()
    {
        return m_targetType;
    }

private:
    WinCmdExector() {}
    int ExecuteCmd(const std::vector<std::string> &cmdParams);
    int ExecuteCmdInner(const std::string &combinStr, DWORD timeOut = INFINITE);
    std::string GenerateCmdStr(const std::vector<std::string> &cmdParams);
    int WriteInfoToParamFile(const std::string &filePath, const Json::Value &param);
    int CheckCmdDelimiter(const std::string& str);
    int WriteParam(Json::Value &inputValue, std::string &uniqueId);
    int WriteFileWithRetry(const std::string &filePath, const std::string &content);
    int DeleteResultFile(const std::string &filePath);
    int GetScriptExecResult(const Param &cmdParam, const std::string &uniqueId, Json::Value &result);
    std::string ConsolidateSenInfo();
    void CleanMemorySenInfo(std::string &senInfo);
    int CreateProcPipe(HANDLE &hStdInputRd, HANDLE &hStdInputWr);
    std::string GetParamFile(const std::string &uuid);
    std::string GetResultFile(const std::string &uuid);

private:
    Param m_param;
    std::string m_targetHost;
    CMD_RUNNING_TARGET m_targetType = CMD_RUNNING_TARGET::TYPE_HOST;
};

} // namespace Utils
} // namespace HyperVPlugin
#endif
#endif