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
#include "CommonDefine.h"

TEST_API bool ExecSystemCmd(const std::string& strCommand, std::vector<std::string>& strEcho)
{
    FILE* pStream = popen(strCommand.c_str(), "r");
    if (NULL == pStream) {
        Log("popen failed.");
        return false;
    }

    while (!feof(pStream)) {
        char tmpBuf[1000] = {0};
        char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (NULL == cRet) {
        }
        if (strlen(tmpBuf) >= 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  
        }

        bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }
        strEcho.push_back(std::string(tmpBuf));
    }

    for (size_t i = 0;i < strEcho.size();++i) {
        Log("command echo is:%s", strEcho[i].c_str());
    }

    (void)pclose(pStream);
    return true;   
}

TEST_API bool CreateNewDir(const std::string &newDir)
{
    std::string strCmd = "mkdir -p " + newDir;

    std::vector<std::string> strEcho;
    int ret = ExecSystemCmd(strCmd, strEcho);
    if (!ret) {
        Log("exec cmd(%s) fail.",  strCmd.c_str());
    }    
    return ret;
}

TEST_API bool CopyFile(const std::string &srcFile, const std::string &dstFile)
{
    std::string strCmd = "cp " + srcFile + " " + dstFile;

    std::vector<std::string> strEcho;
    int ret = ExecSystemCmd(strCmd, strEcho);
    if (!ret) {
        Log("exec cmd(%s) fail.",  strCmd.c_str());
    }    
    return ret;   
}
TEST_API std::string GenerateSerial()
{
    time_t tt; 
    time( &tt ); 
    tm* t= gmtime( &tt ); 
    char timeStr[100]; 
    memset(timeStr, 0, 100); 
    sprintf(timeStr, "%d%02d%02d_%02d%02d%02d", t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec); 
    int tempPid = getpid(); 
    std::string serial = std::to_string(tempPid) + "_" + std::string(timeStr);
    return serial;      
}
TEST_API bool DeleteFile(const std::string &fileName)
{
    std::string strCmd = "rm " + fileName;

    std::vector<std::string> strEcho;
    int ret = ExecSystemCmd(strCmd, strEcho);
    if (!ret) {
        Log("exec cmd(%s) fail.",  strCmd.c_str());
    }    
    return ret;      
}
