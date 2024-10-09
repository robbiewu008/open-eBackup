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
#ifndef __DB_PLUGIN_PATH_H__
#define __DB_PLUGIN_PATH_H__

#include <memory>
#include "define/Types.h"
#include "define/Defines.h"
#include "common/Defines.h"

namespace GeneralDB {
#ifndef WIN32
// 参数文件路径
const mp_string GENERALDB_PARAM_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp/";
// 结果文件路径
const mp_string GENERALDB_RESULT_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp/";
// 配置文件路径
const mp_string GENERALDB_CONF_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/";
// 分解任务配置文件
const mp_string APPLICATION_GENSUB_CONF_PATH =
    "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/GenSub.conf";
// 应用脚本文件路径
const mp_string GENERALDB_SCRIPT_PATH =
        "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/applications/";
const mp_string AGENT_NGINX_CONF_PATH = "/DataBackup/ProtectClient/ProtectClient-E/nginx/conf/";
#else
const mp_string GENERALDB_PARAM_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp/";
const mp_string GENERALDB_RESULT_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp/";
const mp_string GENERALDB_CONF_PATH = "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/";
const mp_string APPLICATION_GENSUB_CONF_PATH =
    "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/conf/GenSub.conf";
const mp_string GENERALDB_SCRIPT_PATH =
        "/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/applications/";
const mp_string AGENT_NGINX_CONF_PATH = "\\DataBackup\\ProtectClient\\ProtectClient-E\\nginx\\conf\\";
#endif

class DBPluginPath {
public:
    static std::shared_ptr<DBPluginPath> GetInstance();
    ~DBPluginPath();
    mp_int32 SetDBPluginPath(const mp_string &logPath);

    mp_string GetParamPath();

    mp_string GetResultPath();

    mp_string GetGeneraldbConfPath();

    mp_string GetGeneraldbGensubPath();

    mp_string GetScriptPath();

    mp_string GetNginxConfPath();

    mp_string GetLogPath();
private:
    mp_int32 SetLogPath(const mp_string &logPath);
    mp_int32 SetInstallHeadPath(const mp_string &logPath);
    mp_int32 SetParamPath();
    mp_int32 SetResultPath();
    mp_int32 SetScriptPath();

private:
    char* m_generalBDLogPath;
    char* m_generalBDParamPath;
    char* m_generalBDResultPath;
    char* m_generalBDScriptPath;
    mp_string m_strAgentHeadPath;
    mp_string m_logPath;
};
}
#endif