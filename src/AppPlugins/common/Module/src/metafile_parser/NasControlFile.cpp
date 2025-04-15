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
#include "NasControlFile.h"
#include "define/Defines.h"
#include "define/Types.h"

#ifdef WIN32
/* Hint: to provide dir exist check */
#include <filesystem>
#endif
using namespace std;

time_t GetCurrentTimeInSeconds()
{
    time_t currTime;
    time(&currTime);
    return currTime;
}

string GetParentDirOfFile(string filePath)
{
    string parentDirPath = filePath.substr(0, filePath.find_last_of("/"));
    return parentDirPath;
}

NAS_CTRL_FILE_RETCODE  CheckParentDirIsReachable(string dirPath)
{
    int retryCnt = 0;
    int ret = -1;
    do {
        retryCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(NAS_CTRL_FILE_SERVER_RETRY_INTERVAL));
#ifdef WIN32
        ret = std::filesystem::is_directory(dirPath) ? 0 : -1; /* msvc support C++17 */
#else
        struct stat64 sb {};
        ret = lstat64(dirPath.c_str(), &sb);
#endif
        if (ret == -1) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "Stat failed for directory: " << dirPath << " retry count: "
                << retryCnt << " ERR: " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        }
    } while ((ret == -1) && (retryCnt < NAS_CTRL_FILE_SERVER_RETRY_CNT));
    if (ret == -1) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "Parent Directory is not reachable, DirPath: " << dirPath
            << " ERR: " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    HCP_Log(INFO, CTL_MOD_NAME) << "Parent Dir is reachable, retryCnt: " << retryCnt << " DirPath: "
        << dirPath;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

uint32_t GetCommaCountOfString(const std::string &strName)
{
    uint32_t cnt = (uint32_t) std::count(strName.begin(), strName.end(), ',');
    return cnt;
}

string ConstructStringName(uint32_t &offset, uint32_t &totCommaCnt, vector<std::string> &lineContents)
{
    uint32_t tempCommaCnt = 0;
    string strName {};

    do {
        strName.append(lineContents[offset]);
        if (tempCommaCnt != totCommaCnt) {
            strName.append(",");
        }
        ++offset;
        ++tempCommaCnt;
    } while (tempCommaCnt <= totCommaCnt);

    return strName;
}