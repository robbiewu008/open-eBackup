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
#include "VssTool.h"
#include <string>
#include <cstring>
#include <sstream>
#include "log/Log.h"

using namespace std;
namespace {
    const std::string LOG_NAME = "vsstool.log";

    const std::string LOG_PATH = R"(C:\\DataBackup\\ProtectClient\\ProtectClient-E\\log\\Plugins\\GeneralDBPlugin)";
    const int LOG_LEVEL = 0;
    const int LOG_COUNT = 100;
    const int LOG_MAX_SIZE = 30;
    const int NUMBER2 = 2;
    const int NUMBER3 = 3;
    const int NUMBER4 = 4;
    const int NUMBER5 = 5;
    const int NUMBER6 = 6;
    const int NUMBER7 = 7;
    const int NUMBER8 = 8;
    const int NUMBER9 = 9;
    const std::string UNC_PREFIX = R"(\\?\)";
} // namespace

void PrintHelp()
{
    std::cout << "VSS Backup and Restore Command Instructions:" << std::endl;
    std::cout << "Enter \"VssBackup\" for Backup" << std::endl;
    std::cout << "Enter \"VssRestore\" for Restore" << std::endl;
}

void InitLog()
{
    Module::CLogger::GetInstance().Init(LOG_NAME.c_str(), LOG_PATH, LOG_LEVEL, LOG_COUNT, LOG_MAX_SIZE);
}

std::vector<std::string> spliteString(const std::string& originStr, char splitor)
{
    std::vector<std::string> result;
    std::stringstream ss(originStr);
    while (ss.good()) {
        std::string substr;
        std::getline(ss, substr, splitor);
        result.push_back(substr);
    }
    return result;
}

int HandleBackup(int argc, const std::vector<std::string>& argv)
{
    HCP_Log(INFO, LOG_NAME) << "Start Full Backup!" << HCPENDLOG;
    if (argc < NUMBER5 || argc > NUMBER7) {
        HCP_Log(ERR, LOG_NAME) << "Input Num Error!" << HCPENDLOG;
        return VSSTOOL_FAILED;
    }
    bool guidEnable = (argc >= NUMBER6);
    bool jobIdEnable = (argc >= NUMBER7);
    VssBackupWrapper backupInstance;
    const string backupType = argv[1];
    std::string targetWriterName = argv[NUMBER2];
    std::string targetMetaPath = argv[NUMBER3];
    std::string volumes = argv[NUMBER4];
    std::string guids = guidEnable ? argv[NUMBER5] : "";
    std::string jobId = jobIdEnable ? argv[NUMBER6] : "";
    std::vector<std::string> volumeList;
    std::vector<std::string> guidList {};
    std::stringstream ss(volumes);
    while (ss.good()) {
        std::string substr;
        std::getline(ss, substr, ',');      // 使用,作为分隔符
        INFOLOG("add vloume:%s", substr.c_str());
        substr = substr + ":\\";
        volumeList.push_back(substr);
    }
    ss.clear();
    if (guidEnable) {
        guidList = spliteString(guids, ',');
    }
    ActualBackupParam param = ActualBackupParam(backupType, targetWriterName, targetMetaPath, jobId);
    bool ret = backupInstance.ActualBackup(param, volumeList, guidList);
    if (!ret) {
        HCP_Log(ERR, LOG_NAME) << "Backup Failed!" << HCPENDLOG;
        return VSSTOOL_FAILED;
    }
    return VSSTOOL_SUCCESS;
}

int HandleRestore(int argc, const std::vector<std::string>& argv)
{
    HCP_Log(INFO, LOG_NAME) << "Start Restore!" << HCPENDLOG;
    if (argc != NUMBER9) {
        ERRLOG("Input params error!");
        return VSSTOOL_FAILED;
    }
    // Usage: VssTool.exe
    // 1. Restore
    // 2. /path/to/meta where bcd and wmd located
    // 3. /path/to/data
    // 4. target_ebd_path (absolute path with target edb file name)
    // 5. target log path
    // 6. origin guid
    // 7. target guid
    // 8. log prefix : like E02
    RestoreParam param;
    param.metaPath = UNC_PREFIX + argv[NUMBER2];
    param.dataPath = UNC_PREFIX + argv[NUMBER3];
    param.targetEdbPath = UNC_PREFIX + argv[NUMBER4];
    param.targetLogPath = UNC_PREFIX + argv[NUMBER5];
    param.originGuid = argv[NUMBER6];
    param.targetGuid = argv[NUMBER7];
    param.logPrefix = argv[NUMBER8];
    INFOLOG("Start Restore , Paramaters: \nmetaPath: %s, \n dataPath: %s, \n targetEdbPath: %s,\n"
        "targetLogPath: %s,\n originGuid: %s,\n targetGuid: %s, \n logPrefix: %s",
        param.metaPath.c_str(),
        param.dataPath.c_str(),
        param.targetEdbPath.c_str(),
        param.targetLogPath.c_str(),
        param.originGuid.c_str(),
        param.targetGuid.c_str(),
        param.logPrefix.c_str());

    VssRestore restoreInstance;
    bool ret = restoreInstance.Restore(param);
    if (!ret) {
        INFOLOG("Restore failed for originGuid : %s", param.originGuid.c_str());
        return VSSTOOL_FAILED;
    }
    return VSSTOOL_SUCCESS;
}

int HandleCheckInterity(int argc, const std::vector<std::string>& argv)
{
    if (argc != NUMBER5) {
        ERRLOG("Input param error, expected 5, actual %d", argc);
        return VSSTOOL_FAILED;
    }
    VssBackupWrapper backupInstance;
    std::string baseNames = argv[NUMBER2];
    std::string logPaths = argv[NUMBER3];
    std::string databaseNames = argv[NUMBER4];
    std::vector<std::string> baseNameList = spliteString(baseNames, ',');
    std::vector<std::string> logPathList = spliteString(logPaths, ',');
    std::vector<std::string> databaseNameList = spliteString(databaseNames, ',');
    if (baseNameList.size() != logPathList.size() ||
        logPathList.size() != databaseNameList.size()) {
        ERRLOG("path size not match!");
        return VSSTOOL_FAILED;
    }
    for (UINT i = 0; i < baseNameList.size(); i++) {
        std::string logPathTmp = UNC_PREFIX + logPathList[i];
        std::string databaseNameTmp = UNC_PREFIX + databaseNameList[i];
        INFOLOG("Check Interity: %s, %s, %s", baseNameList[i].c_str(), logPathTmp.c_str(),
            databaseNameTmp.c_str());
        if (!backupInstance.CheckBackupInterity(baseNameList[i], logPathTmp, databaseNameTmp)) {
            ERRLOG("Check Interity failed!%s, %s, %s", baseNameList[i].c_str(), logPathTmp.c_str(),
                databaseNameTmp.c_str());
            return VSSTOOL_FAILED;
        }
    }
    INFOLOG("finish check interity");
    return VSSTOOL_SUCCESS;
}

int main(int argc, char *argv[])
{
    InitLog();

    if (argc < LEAST_VSSTOOL_INPUT_COUNT) {
        HCP_Log(ERR, LOG_NAME) << "Not Enough Input!" << HCPENDLOG;
        return VSSTOOL_FAILED;
    }
    std::vector<std::string> argList{"VssTool.exe"};
    for (int i = 1; i < argc; i++) {
        argList.push_back(std::string(argv[i]));
    }
    if (strcmp(argv[1], "Backup") == 0 || strcmp(argv[1], "Log") == 0) {
        return HandleBackup(argc, argList);
    } else if (strcmp(argv[1], "Restore") == 0) {
        return HandleRestore(argc, argList);
    } else if (strcmp(argv[1], "Delete") == 0) {
        if (argc != DELETE_SNAPSHOT_INPUT_COUNT) {
            HCP_Log(ERR, LOG_NAME) << "Input Num Error!" << HCPENDLOG;
            return VSSTOOL_FAILED;
        }
        VssBackupWrapper backupInstance;
        std::string snapshotId = argv[NUMBER2];
        std::wstring wSnapshotId = backupInstance.Utf8ToUtf16(snapshotId);
        VSS_ID vssSnapshotId = backupInstance.VssIDfromWStr(wSnapshotId);
        CHECK_BOOL_RETURN(backupInstance.DoDeleteSnapshot(vssSnapshotId), "DoDeleteSnapshot", VSSTOOL_FAILED);
        return VSSTOOL_SUCCESS;
    } else if (strcmp(argv[1], "CheckInterity") == 0) {
        return HandleCheckInterity(argc, argList);
    }

    return VSSTOOL_FAILED;
}
