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
#include "logProxy.h"
#include <fstream>
#include <string>
#include <vector>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace {
    constexpr auto ENV_NODE_NAME = "NODE_NAME";
    std::string g_strKmcLogFile = "";
    const std::vector<std::string> BLOCK_LIST = {";", "|", "&", ">", "<", "`", "!", "\\", "\n", "$"};
}

LogProxyKmc::~LogProxyKmc()
{
    std::string msg = str().substr(1);
    std::stringstream ss;

    ss << "[" << m_funcName << "]"
       << "[" << m_line << "]" << msg;
    std::string result = ss.str();
    ofstream fsKmcLog(g_strKmcLogFile, ios::app);
    fsKmcLog << result << endl;
    fsKmcLog.close();
    std::string sysCmd = "chmod 640 " + g_strKmcLogFile;
    system(sysCmd.c_str());
}

bool CheckFileName(const std::string &moduleName)
{
    for (const auto &spec : BLOCK_LIST) {
        if (moduleName.find(spec) != std::string::npos) {
            HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Invalid name, please check your name:"
                << moduleName << g_strKmcLogFile;
            return false;
        }
    }
    return true;
}

/*
 * Function: bool InitKmcLog(std::string moduleName)
 * Description: 日志模块初始化接口
 * Called By: InitKMCV1c   InitKMCV3c
 * Input:  std::string moduleName  不能带特殊字符
 * Output: nullptr
 * Return: ture/false
 */
bool InitKmcLog(const std::string &moduleName)
{
    if (!CheckFileName(moduleName)) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Check file name failed!" << g_strKmcLogFile;
        return false;
    }
    std::string ucNodeStr;
    /* step1: 获取节点信息 */
    char *ucNodeVar = getenv(ENV_NODE_NAME);
    if (ucNodeVar != nullptr) {
        if (!CheckFileName(std::string(ucNodeVar))) {
            HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Check node name failed!" << g_strKmcLogFile;
            return false;
        } else {
            ucNodeStr = std::string(ucNodeVar) +"/kmc-log/";
        }
    } else {
        ucNodeStr = "kmc-log/";
    }
    /* step2: 拼接路径及文件名 */
    std::string ucLogPath;
    if (access("/opt/logpath", F_OK) == 0) {
        ucLogPath = "/opt/logpath/logs/";
    } else {
        ucLogPath = "/opt/DataBackup/logs/";
    }
    std::string ucLogFile = ucLogPath + ucNodeStr + moduleName + ".log";
    /* step3: 检查文件，或者创建文件 */
    if (access(ucLogFile.c_str(), F_OK) != 0) {
        std::vector<std::string> strEcho;
        std::string sysCmd = "mkdir -pv " + ucLogPath + ucNodeStr;
        system(sysCmd.c_str());
        sysCmd = "touch " + ucLogFile;
        system(sysCmd.c_str());
        sysCmd = "chmod 640 " + ucLogFile;
        system(sysCmd.c_str());
        /* 配置属主为：99(nobody) */
        sysCmd = "chown -R 99:99 " + ucLogPath + ucNodeStr;
        system(sysCmd.c_str());
    }
    g_strKmcLogFile = ucLogFile;
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "InitKmcLog Sucessfull,LogFile:" << g_strKmcLogFile;
    return true;
}

#ifdef __cplusplus
}
#endif

