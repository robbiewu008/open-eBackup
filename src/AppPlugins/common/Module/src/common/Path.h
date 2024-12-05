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
#ifndef MODULE_PATH_H
#define MODULE_PATH_H

#include "define/Types.h"
#include "define/Defines.h"

#ifdef WIN32
#include <shlwapi.h>
#endif

namespace Module {

// 路径相关
class AGENT_API CPath {
public:
#ifndef WIN32
    static CPath& GetInstance()
    {
        return m_instance;
    }
#else
    static CPath& GetInstance();
#endif

    ~CPath()
    {}

    // pszBinFilePath的格式为/home/rdadmin/Agent/bin/rdagent
    int Init(const std::string& pszBinFilePath);
    // SetRootPath�ṩͨ��AgentRoot·��֧�ֳ�ʼ���ķ�ʽ
    void SetRootPath(const std::string& strRootPath)
    {
        m_strAgentRootPath = strRootPath;
    }
    // 获取agent路径，如安装路径为/home/rdagent/agent，则返回/home/rdagent/agent
    std::string GetRootPath()
    {
        return m_strAgentRootPath;
    }
    // 获取agent安装路径下的bin文件夹路径
    std::string GetBinPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR;
    }
    // 获取agent安装路径下的sbin文件夹路径
    std::string GetSBinPath()
    {
        if (m_strAgentRootPath[m_strAgentRootPath.size()-1] == '/') {
            return m_strAgentRootPath + AGENT_SBIN_DIR;
        } else {
            return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SBIN_DIR;
        }
    }
    // 获取插件路径
    std::string GetPluginsPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_PLUGIN_DIR;
    }
    std::string GetPluginInstallPath()
    {
        return m_strAgentRootPath + std::string("/../")  + AGENT_PLUGIN_INSTALL_DIR;
    }

    std::pair<int, std::string> GetAbsolutePath(std::string convertPath)
    {
#ifdef WIN32
        char path[MAX_FULL_PATH_LEN + 1] = { 0x00 };
        if (PathCanonicalizeA(path, convertPath.c_str()) == FALSE) {
            return std::make_pair(FAILED, "");
        }
#else
        char path[PATH_MAX + 1] = { 0x00 };
        if (realpath(convertPath.c_str(), path) == NULL) {
            return std::make_pair(FAILED, "");
        }
#endif
        return std::make_pair(SUCCESS, std::string(path, strlen(path)));
    }
    // 获取agent安装路径下的conf文件夹路径
    std::string GetConfPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_CONF_DIR;
    }

    std::string GetAgentConfPath()
    {
        int pos = m_strAgentRootPath.rfind(AGENT_PLUGIN_INSTALL_DIR);
        if (pos == std::string::npos) {
            return "";
        }
        std::string protectClientPath = m_strAgentRootPath.substr(0, pos);
        return protectClientPath + PATH_SEPARATOR + AGENT_PROTECT_CLIENT_E_DIR + PATH_SEPARATOR + AGENT_CONF_DIR;
    }

    std::string GetAgentConfFilePath(const std::string& strFileName)
    {
        return GetAgentConfPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetPluginPidPath()
    {
        return GetConfPath() + PATH_SEPARATOR + "PluginPid";
    }

    std::string GetBackupConfPath()
    {
        return "./conf";
    }

    // 获取agent安装路径下的log文件夹路径(属组为rdadmin)
    std::string GetLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR;
    }
    // 获取agent安装路径下的slog文件夹路径(属组为root)
    std::string GetSlogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SLOG_DIR;
    }
    // 获取给外部插件分配的日志目录(属组为rdadmin)
    std::string GetExPluginLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    }
    // 获取给外部插件分配的日志目录(属组为root)
    std::string GetExPluginSlogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SLOG_DIR + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    }
    std::string GetLibPath() {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LIB_DIR;
    }
    std::string GetDatamoverLibPath() {
        // return absolute datamover lib path
        return "./lib";
    }

    // 获取agent安装路径下的tmp文件夹路径
    std::string GetTmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_TMP_DIR;
    }
    // 获取agent安装路径下的tmp文件夹路径
    std::string GetStmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_STMP_DIR;
    }
    // 获取agent安装路径下的ebk_user文件夹路径
    std::string GetEbkUserPath()
    {
        return GetSBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR + PATH_SEPARATOR + AGENT_EBKUSER_DIR;
    }
    // 获取agent安装路径下的bin文件夹下某个文件路径
    std::string GetThirdPartyPath()
    {
        return GetSBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR;
    }

    std::string GetDbPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_DB;
    }

    std::string GetNginxPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_NGINX;
    }

    std::string GetBinFilePath(std::string strFileName)
    {
        return GetBinPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetSBinFilePath(std::string strFileName)
    {
        return GetSBinPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetConfFilePath(std::string strFileName)
    {
        return GetConfPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetLogFilePath(std::string strFileName)
    {
        return GetLogPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetSlogFilePath(std::string strFileName)
    {
        return GetSlogPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetTmpFilePath(std::string strFileName)
    {
        return GetTmpPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetStmpFilePath(std::string strFileName)
    {
        return GetStmpPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetDbFilePath(std::string strFileName)
    {
        return GetDbPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetThirdPartyFilePath(std::string strFileName)
    {
        return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetNginxFilePath(std::string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + strFileName;
    }

    std::string GetNginxLogsFilePath(std::string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_LOGS + PATH_SEPARATOR + strFileName;
    }

    std::string GetNginxConfFilePath(std::string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + strFileName;
    }

    std::string GetThirdPartyFilePath(std::string strFileName, std::string isUserDefined)
    {
        if (isUserDefined == AGENT_USER_DEFINED_SCRIPT) {
            return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
        } else if (isUserDefined == AGENT_SAMPLE_SCRIPT) {
            return GetThirdPartyPath() + PATH_SEPARATOR + std::string(AGENT_USER_DEFINED_DIR) + PATH_SEPARATOR +
                   strFileName;
        }
        return "";
    }

private:
    CPath()
    {}

private:
    static CPath m_instance;
    std::string m_strAgentRootPath;
};

} // namespace Module

#endif  // AGENT_PATH_H
