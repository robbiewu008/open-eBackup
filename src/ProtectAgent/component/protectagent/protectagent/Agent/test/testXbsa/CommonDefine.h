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
#ifndef _COMMON_DEFINE_H_
#define _COMMON_DEFINE_H_
#include <iostream>
#include <time.h>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <vector>

#define TEST_API
enum BUSINESS_TYPE
{
    Backup = 0,
    Recover = 1, 
    Delete = 2
};

const std::string TEST_BACKUP_DIR = "/home/rdadmin/test/backup/";
const std::string TEST_DELETE_DIR = "/home/rdadmin/test/delete/";
const std::string TEST_RECOVER_DIR = "/home/rdadmin/test/recover/";

#define Log(format, args...) \
{\
    time_t tt; \
    time( &tt ); \
    tm* t= gmtime( &tt ); \
    char timeStr[100]; \
    memset(timeStr, 0, 100); \
    sprintf(timeStr, "%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec); \
    int tempPid = getpid(); \
    std::cout << std::string(timeStr) << " [" << tempPid << "]  ";\
    printf(format, ##args);\
    std::cout << "\n"; \
}

TEST_API bool ExecSystemCmd(const std::string& strCommand, std::vector<std::string>& strEcho);

TEST_API bool CreateNewDir(const std::string &newDir);

TEST_API bool CopyFile(const std::string &srcFile, const std::string &dstFile);

TEST_API std::string GenerateSerial();
TEST_API bool DeleteFile(const std::string &fileName);
#endif