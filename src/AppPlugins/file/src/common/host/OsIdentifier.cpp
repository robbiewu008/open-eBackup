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
#include <iostream>
#include <fstream>
#include <string>

#include "utils/PluginUtilities.h"

#ifndef _WIN32
#include <sys/utsname.h>
#endif

#include "OsIdentifier.h"

using namespace FilePlugin;

#ifdef WIN32
// read os info using win32 api, implement later
std::string OsIdentifier::GetSystemName()
{
    return "Windows";
}

std::string OsIdentifier::GetSystemVersion()
{
    return "Windows Version Unknown";
}

#else
// read os info using posix api (Linux/AIX/Solaris)
std::string OsIdentifier::GetSystemName()
{
    struct utsname buf {};
    if (::uname(&buf) < 0) {
        ERRLOG("failed to call uname, error %d", errno);
        return "";
    }
    return std::string(buf.sysname);
}

std::string OsIdentifier::GetSystemVersion()
{
    struct utsname buf {};
    if (::uname(&buf) < 0) {
        ERRLOG("failed to call uname, error %d", errno);
        return "";
    }
    return std::string(buf.version);
}

#endif

bool OsIdentifier::CreateSystemNameRecordFile(const std::string& dirPath)
{
    std::string systemName = OsIdentifier::GetSystemName();
    if (systemName.empty()) {
        return false;
    }
    std::string fullpath = PluginUtils::PathJoin(dirPath, OS_IDENTIFIER_SYSTEM_NAME_RECORD_FILE_NAME);
    INFOLOG("save system name %s to record file %s", systemName.c_str(), fullpath.c_str());
    try {
        std::ofstream outFileStream(fullpath, std::ios::out | std::ios::trunc);
        if (!outFileStream.is_open()) {
            ERRLOG("open file %s failed, errno %d", fullpath.c_str(), errno);
            return false;
        }
        outFileStream << systemName;
        outFileStream.close();
    } catch (...) {
        ERRLOG("create system name record got exception");
        return false;
    }
    return true;
}

bool OsIdentifier::CreateSystemVersionRecordFile(const std::string& dirPath)
{
    std::string systemVersion = OsIdentifier::GetSystemVersion();
    if (systemVersion.empty()) {
        return false;
    }
    std::string fullpath = PluginUtils::PathJoin(dirPath, OS_IDENTIFIER_SYSTEM_VERSION_RECORD_FILE_NAME);
    INFOLOG("save system version %s to record file %s", fullpath.c_str(), fullpath.c_str());
    try {
        std::ofstream outFileStream(fullpath, std::ios::out | std::ios::trunc);
        if (!outFileStream.is_open()) {
            ERRLOG("open file %s failed, errno %d", fullpath.c_str(), errno);
            return false;
        }
        outFileStream << systemVersion;
        outFileStream.close();
    } catch (...) {
        ERRLOG("create system version record got exception");
        return false;
    }
    return true;
}

std::string OsIdentifier::ReadSystemName(const std::string& dirPath)
{
    std::string fullpath = PluginUtils::PathJoin(dirPath, OS_IDENTIFIER_SYSTEM_NAME_RECORD_FILE_NAME);
    std::string systemName;
    INFOLOG("read system name from record file %s", fullpath.c_str());
    try {
        std::ifstream inFileStream {};
        std::stringstream fileBuffer {};
        inFileStream.open(fullpath.c_str(), std::ios::in);
        if (!inFileStream.is_open()) {
            ERRLOG("read from file open file failed %s, errno %d", fullpath.c_str(), errno);
            return "";
        }
        fileBuffer << inFileStream.rdbuf();
        systemName = fileBuffer.str();
        inFileStream.close();
    } catch (...) {
        ERRLOG("read system name record file got exception");
    }
    return systemName;
}

std::string OsIdentifier::ReadSystemVersion(const std::string& dirPath)
{
    std::string fullpath = PluginUtils::PathJoin(dirPath, OS_IDENTIFIER_SYSTEM_VERSION_RECORD_FILE_NAME);
    std::string systemVersion;
    INFOLOG("read system version from record file %s", fullpath.c_str());
    try {
        std::ifstream inFileStream {};
        std::stringstream fileBuffer {};
        inFileStream.open(fullpath.c_str(), std::ios::in);
        if (!inFileStream.is_open()) {
            ERRLOG("read from file open file failed %s, errno %d", fullpath.c_str(), errno);
            return "";
        }
        fileBuffer << inFileStream.rdbuf();
        systemVersion = fileBuffer.str();
        inFileStream.close();
    } catch (...) {
        ERRLOG("read system version record file got exception");
    }
    return systemVersion;
}


#ifdef SOLARIS

bool OsIdentifier::CheckSolarisVersionCompatible(const std::string& dirPath)
{
    std::string localSystemName = OsIdentifier::GetSystemName();
    std::string localSystemVersion = OsIdentifier::GetSystemVersion();
    std::string copySystemName = OsIdentifier::ReadSystemName(dirPath);
    std::string copySystemVersion = OsIdentifier::ReadSystemVersion(dirPath);
    INFOLOG("local system : %s %s, copy system %s %s",
        localSystemName.c_str(), localSystemVersion.c_str(), copySystemName.c_str(), copySystemVersion.c_str());
    if (localSystemName != copySystemName) {
        return false;
    }
    if (localSystemVersion < copySystemVersion) {
        return false;
    }
    return true;
}

#endif
