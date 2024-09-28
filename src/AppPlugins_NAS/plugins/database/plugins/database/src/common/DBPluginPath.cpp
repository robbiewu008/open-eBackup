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
#include <algorithm>
#include "log/Log.h"
#include "common/File.h"
#include "securec.h"
#include "DBPluginPath.h"


using namespace GeneralDB;

namespace {
    const mp_string MODULE = "DBPluginPath";
    const mp_string AGENT_INSTALL_PATH = "DataBackup";
    const mp_string GENERALDB_ROOT_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin";
}

std::shared_ptr<DBPluginPath> DBPluginPath::GetInstance()
{
    static std::shared_ptr<DBPluginPath> g_instance = std::make_shared<DBPluginPath>();
    return g_instance;
}

DBPluginPath::~DBPluginPath()
{
    if (m_generalBDLogPath) {
        delete[] m_generalBDLogPath;
        m_generalBDLogPath = nullptr;
    }
    if (m_generalBDParamPath) {
        delete[] m_generalBDParamPath;
        m_generalBDParamPath = nullptr;
    }
    if (m_generalBDResultPath) {
        delete[] m_generalBDResultPath;
        m_generalBDResultPath = nullptr;
    }
    if (m_generalBDScriptPath) {
        delete[] m_generalBDScriptPath;
        m_generalBDScriptPath = nullptr;
    }
}

mp_int32 DBPluginPath::SetDBPluginPath(const mp_string &logPath)
{
    if (SetInstallHeadPath(logPath) != MP_SUCCESS) {
        return MP_FAILED;
    }
    if (SetLogPath(logPath) != MP_SUCCESS) {
        return MP_FAILED;
    }
    if (SetParamPath() != MP_SUCCESS) {
        return MP_FAILED;
    }
    if (SetResultPath() != MP_SUCCESS) {
        return MP_FAILED;
    }
    return SetScriptPath();
}

mp_int32 DBPluginPath::SetInstallHeadPath(const mp_string &logPath)
{
    const mp_char repliceSeparates = '\\';
    const mp_char targetSeparates = '/';
    mp_string envPath = getenv("DATA_BACKUP_AGENT_HOME");
    replace(envPath.begin(), envPath.end(), repliceSeparates, targetSeparates);
    m_strAgentHeadPath = envPath;
    if (m_strAgentHeadPath.empty()) {
        ERRLOG("Faild to get agent install head path.");
        return MP_FAILED;
    }
    mp_string GeneraldbRootPath = m_strAgentHeadPath + GENERALDB_ROOT_PATH;
    if (!Module::CFile::DirExist(GeneraldbRootPath.c_str())) {
        ERRLOG("Generaldb root path is not exists");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 DBPluginPath::SetLogPath(const mp_string &logPath)
{
    HCP_Log(INFO, MODULE) << "SetLogPath." << HCPENDLOG;
    if (logPath.empty()) {
        HCP_Log(ERR, MODULE) << "Log path is empty." << HCPENDLOG;
        return MP_FAILED;
    }
    m_logPath = logPath;
    mp_string logVar = std::string("GENERALDB_LOG_PATH") + "=" + logPath;
    m_generalBDLogPath = new char[logVar.length() + 1];
    if (strcpy_s(m_generalBDLogPath, logVar.length() + 1, logVar.c_str()) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Copy log path env variable failed." << HCPENDLOG;
        return MP_FAILED;
    }
    putenv(m_generalBDLogPath);
    return MP_SUCCESS;
}

mp_string DBPluginPath::GetLogPath()
{
    return m_logPath;
}

mp_string DBPluginPath::GetParamPath()
{
    return m_strAgentHeadPath + GENERALDB_PARAM_PATH;
}

mp_string DBPluginPath::GetResultPath()
{
    return m_strAgentHeadPath + GENERALDB_RESULT_PATH;
}

mp_string DBPluginPath::GetGeneraldbConfPath()
{
    return m_strAgentHeadPath + GENERALDB_CONF_PATH;
}

mp_string DBPluginPath::GetGeneraldbGensubPath()
{
    return m_strAgentHeadPath + APPLICATION_GENSUB_CONF_PATH;
}

mp_string DBPluginPath::GetScriptPath()
{
    return m_strAgentHeadPath + GENERALDB_SCRIPT_PATH;
}

mp_string DBPluginPath::GetNginxConfPath()
{
    return m_strAgentHeadPath + AGENT_NGINX_CONF_PATH;
}

mp_int32 DBPluginPath::SetParamPath()
{
    mp_string paramVar = std::string("GENERALDB_PARAM_PATH") + "=" + GetParamPath();
    m_generalBDParamPath = new char[paramVar.length() + 1];
    if (strcpy_s(m_generalBDParamPath, paramVar.length() + 1, paramVar.c_str()) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Copy param path env variable failed." << HCPENDLOG;
        return MP_FAILED;
    }
    putenv(m_generalBDParamPath);
    return MP_SUCCESS;
}

mp_int32 DBPluginPath::SetResultPath()
{
    mp_string resultVar = std::string("GENERALDB_RESULT_PATH") + "=" + GetResultPath();
    m_generalBDResultPath = new char[resultVar.length() + 1];
    if (strcpy_s(m_generalBDResultPath, resultVar.length() + 1, resultVar.c_str()) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Copy result path env variable failed." << HCPENDLOG;
        return MP_FAILED;
    }
    putenv(m_generalBDResultPath);
    return MP_SUCCESS;
}

mp_int32 DBPluginPath::SetScriptPath()
{
    mp_string scriptVar = std::string("PYTHONPATH") + "=" + GetScriptPath();
    m_generalBDScriptPath = new char[scriptVar.length() + 1];
    if (strcpy_s(m_generalBDScriptPath, scriptVar.length() + 1, scriptVar.c_str()) != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Copy python path env variable failed." << HCPENDLOG;
        return MP_FAILED;
    }
    putenv(m_generalBDScriptPath);
    return MP_SUCCESS;
}
