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
#include <sys/stat.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

#ifdef __cplusplus
extern "C" {
#endif

namespace {
    constexpr auto ENV_NODE_NAME = "NODE_NAME";
    std::string g_strKmcLogFile = "";
    const std::vector<std::string> BLOCK_LIST = {";", "|", "&", ">", "<", "`", "!", "\\", "\n", "$"};
    const uint32_t MB_TO_BYTE = 1048576;
    const uint32_t MAX_SIZE = 10 * MB_TO_BYTE;
    const int SUCCESS { 0 };
    const int FAILED { -1 };
    const std::string LOG_SUFFIX = ".log";
    const std::string ZIP_LOG_SUFFIX = ".gz";
    const std::string DOT = ".";
    const std::string TMP = "Tmp_";
    const uint32_t MAX_KEEP_TIME {15552000};
    const int DEFAULT_LOG_COUNT {20};
}

LogProxyKmc::~LogProxyKmc()
{
    StoreLogFile();
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

void LogProxyKmc::StoreLogFile()
{
    uint32_t fileSize = 0;
    GetFileSize(g_strKmcLogFile.c_str(), fileSize);

    if (fileSize >= MAX_SIZE) {
        std::string timeStamp = to_string(static_cast<uint64_t>(
            duration_cast<seconds>(system_clock::now().time_since_epoch()).count()));
        string tmpLogFile =
            g_strKmcLogFile.substr(0, g_strKmcLogFile.length() - LOG_SUFFIX.length()) + TMP + timeStamp + LOG_SUFFIX;
        rename(g_strKmcLogFile.c_str(), tmpLogFile.c_str());
        ChmodFile(tmpLogFile, S_IRUSR);
        std::thread th { &LogProxyKmc::AsyncZipLogFile, this, tmpLogFile};
        th.detach();
    }
    WriteToLogFile();
}

bool LogProxyKmc::GetFileSize(const char* filePath, uint32_t& size)
{
    if (filePath == nullptr) {
        return false;
    }
    struct stat fileSizeStat;
    if (0 != stat(filePath, &fileSizeStat)) {
        return false;
    }

    size = fileSizeStat.st_size;

    return true;
}

void LogProxyKmc::WriteToLogFile()
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

bool LogProxyKmc::FileExist(const std::string& filePath)
{
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        return false;
    }

    // 目录返回false
    if (S_ISDIR(fileStat.st_mode)) {
        return false;
    }

    return true;
}

bool LogProxyKmc::ZipLogFile(const std::string& filePath)
{
    string zipCmd;
    string zipLogName;

    zipLogName = filePath + ".gz";
    zipCmd = string("gzip -f -q -9 \"") + filePath + "\"";

    int iRet = system(zipCmd.c_str());
    if (!WIFEXITED(iRet)) {
        return false;
    }
    (void)ChmodFile(zipLogName, S_IRUSR);
    return true;
}

int LogProxyKmc::ChmodFile(std::string strFileName, uint32_t mode)
{
    struct stat buf;
    int iRet = lstat(strFileName.c_str(), &buf);
    if (iRet != SUCCESS) {
        return FAILED;
    }
    if (S_ISLNK(buf.st_mode)) {
        return FAILED;
    }
    iRet = chmod(strFileName.c_str(), mode_t(mode));
    if (iRet != SUCCESS) {
        return FAILED;
    }
    return iRet;
}

void LogProxyKmc::AsyncZipLogFile(std::string tmpLogFile)
{
    for (int LogNum = DEFAULT_LOG_COUNT; LogNum > 0; LogNum--) {
        std::string oldLogZipName = g_strKmcLogFile + DOT + to_string(LogNum) + ZIP_LOG_SUFFIX;
        if (FileExist(oldLogZipName)) {
            if (CheckExpired(oldLogZipName)) {
                continue;
            }

            if (LogNum == DEFAULT_LOG_COUNT) {
                remove(oldLogZipName.c_str());
            }

            std::string newLogZipName = g_strKmcLogFile + DOT + to_string(LogNum + 1) + ZIP_LOG_SUFFIX;
            rename(oldLogZipName.c_str(), newLogZipName.c_str());
            ChmodFile(newLogZipName, S_IRUSR);
        }
    }
    std::string finalZipLogFile = g_strKmcLogFile + DOT + "1" + ZIP_LOG_SUFFIX;
    std::string curZipLogFile = tmpLogFile + ZIP_LOG_SUFFIX;
    ZipLogFile(tmpLogFile);
    rename(curZipLogFile.c_str(), finalZipLogFile.c_str());
    ChmodFile(finalZipLogFile, S_IRUSR);
}

bool LogProxyKmc::CheckExpired(std::string logFile)
{
    time_t LastModifyTime;
    time_t NowTime;
    time_t KeepTime;

    time(&NowTime);
    int iRet = GetlLastModifyTime(logFile.c_str(), LastModifyTime);
    if (iRet != SUCCESS) {
        return false;
    }
    if (NowTime < LastModifyTime) {
        return false;
    }
    KeepTime = NowTime - LastModifyTime;
    if (KeepTime > MAX_KEEP_TIME) {
        remove(logFile.c_str());
        return true;
    }
    return false;
}

int LogProxyKmc::GetlLastModifyTime(const char* FilePath, time_t& lastModifyTime)
{
    if (FilePath == nullptr) {
        return FAILED;
    }

    struct stat fileStat;
    if (0 != stat(FilePath, &fileStat)) {
        return FAILED;
    }

    lastModifyTime = fileStat.st_mtime;

    return SUCCESS;
}

#ifdef __cplusplus
}
#endif

