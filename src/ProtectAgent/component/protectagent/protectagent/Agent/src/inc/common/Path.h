/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Path.h
 * @brief  The implemention Path
 * @version 1.0.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */
#ifndef __AGENT_PATH_H__
#define __AGENT_PATH_H__

#include "common/Defines.h"
#include "common/Types.h"

// 路径相关
class AGENT_API CPath {
public:
    static CPath& GetInstance();

    ~CPath()
    {}

    // pszBinFilePath的格式为/home/rdadmin/Agent/bin/rdagent
    mp_int32 Init(const mp_string& pszBinFilePath);
    // SetRootPath提供通过AgentRoot路径支持初始化的方式
    mp_void SetRootPath(const mp_string& strRootPath)
    {
        m_strAgentRootPath = strRootPath;
    }
    // 获取agent路径，如安装路径为/home/rdagent/agent，则返回/home/rdagent/agent
    mp_string GetRootPath()
    {
        return m_strAgentRootPath;
    }
    // 获取agent安装路径下的bin文件夹路径
    mp_string GetBinPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR;
    }
    // 获取agent安装路径下的sbin文件夹路径
    mp_string GetSBinPath()
    {
        if (m_strAgentRootPath[m_strAgentRootPath.size()-1] == '/') {
            return m_strAgentRootPath + AGENT_SBIN_DIR;
        } else {
            return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SBIN_DIR;
        }
    }
    // 获取插件路径
    mp_string GetPluginsPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_PLUGIN_DIR;
    }
    mp_string GetPluginInstallPath()
    {
        return m_strAgentRootPath + mp_string("/../")  + AGENT_PLUGIN_INSTALL_DIR;
    }
    
    // 获取agent安装路径下的conf文件夹路径
    mp_string GetConfPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_CONF_DIR;
    }
    // 获取openv路径下的conf文件夹路径
    mp_string GetXBSAConfPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + XBSA_CONF_DIR;
    }
    // 获取openv路径下的log文件夹路径
    mp_string GetXBSALogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + XBSA_LOG_DIR;
    }
    // 获取openv路径下的conf文件夹下某个文件路径
    mp_string GetXBSAConfFilePath(const mp_string& strFileName)
    {
        return GetXBSAConfPath() + PATH_SEPARATOR + strFileName;
    }
    
    mp_string GetPluginPidPath()
    {
        return GetConfPath() + PATH_SEPARATOR + "PluginPid";
    }

    mp_string GetBackupConfPath()
    {
        return "./conf";
    }
	
    // 获取agent安装路径下的log文件夹路径(属组为rdadmin)
    mp_string GetLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR;
    }
    // 获取agent安装路径下的slog文件夹路径(属组为root)
    mp_string GetSlogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SLOG_DIR;
    }
    // 获取给外部插件分配的日志目录(属组为rdadmin)
    mp_string GetExPluginLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    }
    // 获取给外部插件分配的日志目录(属组为root)
    mp_string GetExPluginSlogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_SLOG_DIR + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    }
    mp_string GetLibPath() {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LIB_DIR;
    }
    mp_string GetDatamoverLibPath() {
        // return absolute datamover lib path
        return "./lib";
    }
	
    // 获取agent安装路径下的tmp文件夹路径
    mp_string GetTmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_TMP_DIR;
    }
    // 获取agent安装路径下的tmp文件夹路径
    mp_string GetStmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_STMP_DIR;
    }
    // 获取agent安装路径下的ebk_user文件夹路径
    mp_string GetEbkUserPath()
    {
        return GetSBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR + PATH_SEPARATOR + AGENT_EBKUSER_DIR;
    }
    // 获取agent安装路径下的thirdparty文件夹路径
    mp_string GetThirdPartyPath()
    {
        return GetSBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR;
    }
    // 获取agent安装路径下的DB文件夹路径
    mp_string GetDbPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_DB;
    }
    // 获取agent安装路径下bin文件夹下nginx子文件夹路径
    mp_string GetNginxPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_NGINX;
    }
    // 获取agent安装路径下的bin文件夹下某个文件路径
    mp_string GetBinFilePath(const mp_string& strFileName)
    {
        return GetBinPath() + PATH_SEPARATOR + strFileName;
    }

    // 获取agent安装路径下的sbin文件夹下某个文件路径
    mp_string GetSBinFilePath(const mp_string& strFileName)
    {
        return GetSBinPath() + PATH_SEPARATOR + strFileName;
    }

    // 获取agent安装路径下的conf文件夹下某个文件路径
    mp_string GetConfFilePath(const mp_string& strFileName)
    {
        return GetConfPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的log文件夹下某个文件路径
    mp_string GetLogFilePath(const mp_string& strFileName)
    {
        return GetLogPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的slog文件夹下某个文件路径
    mp_string GetSlogFilePath(const mp_string& strFileName)
    {
        return GetSlogPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的tmp文件夹下某个文件路径
    mp_string GetTmpFilePath(const mp_string& strFileName)
    {
        return GetTmpPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的stmp文件夹下某个文件路径
    mp_string GetStmpFilePath(const mp_string& strFileName)
    {
        return GetStmpPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的DB文件夹下某个文件路径
    mp_string GetDbFilePath(const mp_string& strFileName)
    {
        return GetDbPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的thirdparty文件夹下某个文件路径
    mp_string GetThirdPartyFilePath(const mp_string& strFileName)
    {
        return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取nginx目录下某个文件的路径
    mp_string GetNginxFilePath(const mp_string& strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + strFileName;
    }
    // 获取nginx logs目录下某个文件的路径
    mp_string GetNginxLogsFilePath(const mp_string& strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_LOGS + PATH_SEPARATOR + strFileName;
    }
    // 获取nginx conf目录下某个文件的路径
    mp_string GetNginxConfFilePath(const mp_string& strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + strFileName;
    }
    // 获取agent安装路径下的thirdparty文件夹下某个文件路径
    mp_string GetThirdPartyFilePath(const mp_string& strFileName, const mp_string& isUserDefined)
    {
        if (isUserDefined == AGENT_USER_DEFINED_SCRIPT) {
            return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
        } else if (isUserDefined == AGENT_SAMPLE_SCRIPT) {
            return GetThirdPartyPath() + PATH_SEPARATOR + mp_string(AGENT_USER_DEFINED_DIR) + PATH_SEPARATOR +
                   strFileName;
        }
        return "";
    }

    // 存放agent升级验签的公钥文件及签名文件目录
    mp_string GetAgentUpgradePath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_UPGRADE;
    }

    // 获取agent安装路径下VDDK文件夹路径
    mp_string GetAgentVDDKPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LIB_DIR + PATH_SEPARATOR + AGENT_VDDK_DIR;
    }

private:
    CPath()
    {}

private:
    static CPath m_instance;  // 单例对象
    mp_string m_strAgentRootPath;
};

#endif  // __AGENT_PATH_H__
