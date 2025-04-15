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
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include <fstream>
#include <vector>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"

#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"

using namespace std;

namespace {
    const mp_int32 KB_TO_BYTE = 1024;
}

mp_string DwsTaskInfoParser::GetBackupCacheInfoPath(const BsaObjectDescriptor &objDesc)
{
    mp_string cacheFilePath;
    if (objDesc.resourceType == "L") {
        cacheFilePath = "/opt/DataBackup/log_cacheInfo.txt";
    } else {
        cacheFilePath = CPath::GetInstance().GetStmpFilePath("data_cacheInfo.txt");
    };
    return cacheFilePath;
}

mp_string DwsTaskInfoParser::GetRestoreCacheInfoPath(const BsaQueryDescriptor &objDesc)
{
    std::vector<mp_string> vecTokens;
    CMpString::StrSplit(vecTokens, (mp_string)(objDesc.objectName.pathName), '/');
    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath("data_cacheInfo.txt");
    return cacheFilePath;
}

mp_int32 DwsTaskInfoParser::ParseCacheInfo(DwsCacheInfo &info, mp_string cacheFilePath)
{
    mp_string readLine;
    mp_int32 ret = ReadFile(cacheFilePath, readLine);
    if (ret != MP_SUCCESS) {
        ERRLOG("Read %s failed!", cacheFilePath.c_str());
        return MP_FAILED;
    }

    if (!JsonHelper::JsonStringToStruct(readLine, info)) {
        ERRLOG("Parse %s failed!readLine=%s.", cacheFilePath.c_str(), readLine.c_str());
        return MP_FAILED;
    }

    if (info.cacheRepoPath.empty() || info.metaRepoPath.empty()) {
        ERRLOG("CacheInfo invalid, cacheRepoPath=%s or metaRepoPath=%s is empty.",
            info.cacheRepoPath.c_str(), info.metaRepoPath.c_str());
        return MP_FAILED;
    }

    if (info.copyId.empty() || info.taskId.empty() || info.hostKey.empty()) {
        ERRLOG("CacheInfo invalid, copyId=%s or taskId=%s or hostKey=%s is empty.",
            info.copyId.c_str(), info.taskId.c_str(), info.hostKey.c_str());
        return MP_FAILED;
    }

    DBGLOG("cacheRepoPath=%s,metaRepoPath=%s,copyId=%s,taskId=%s,hostKey=%s.",
        info.cacheRepoPath.c_str(), info.metaRepoPath.c_str(),
        info.copyId.c_str(), info.taskId.c_str(), info.hostKey.c_str());
    return MP_SUCCESS;
}

mp_int32 DwsTaskInfoParser::ParseTaskInfo(const DwsCacheInfo &cacheInfo, DwsTaskInfo &taskInfo)
{
    mp_string readLine;
    const mp_string taskFilePath = cacheInfo.cacheRepoPath + "/tmp/"
        + cacheInfo.copyId + "/taskInfo_" + cacheInfo.hostKey + ".txt";
    mp_int32 ret = ReadFile(taskFilePath, readLine);
    if (ret != MP_SUCCESS) {
        ERRLOG("Read %s failed!", taskFilePath.c_str());
        return MP_FAILED;
    }

    if (!JsonHelper::JsonStringToStruct(readLine, taskInfo)) {
        ERRLOG("Parse %s failed!readLine=%s.", taskFilePath.c_str(), readLine.c_str());
        return MP_FAILED;
    }

    INFOLOG("Parse %s success!", taskFilePath.c_str());

    if (!CheckTaskInfo(taskInfo)) {
        ERRLOG("Check task info failed!readLine=%s.", readLine.c_str());
        return MP_FAILED;
    }

    INFOLOG("Parse task info success!taskType=%d,copyType=%u.", taskInfo.taskType, taskInfo.copyType);
    return MP_SUCCESS;
}

bool DwsTaskInfoParser::CheckTaskInfo(const DwsTaskInfo &taskInfo)
{
    static std::set<mp_uint32> validTaskTypes = {
        static_cast<mp_uint32>(DwsTaskType::BACKUP),
        static_cast<mp_uint32>(DwsTaskType::RESTORE),
        static_cast<mp_uint32>(DwsTaskType::DELETE)
    };
    if (validTaskTypes.count(taskInfo.taskType) == 0) {
        ERRLOG("Invalid task type=%d", taskInfo.taskType);
        return false;
    }

    if (taskInfo.copyType < CopyDataType::type::FULL_COPY || taskInfo.copyType > CopyDataType::type::CLONE_COPY) {
        ERRLOG("Invalid copy type=%u.", taskInfo.copyType);
        return false;
    }

    if (!CheckRepositories(taskInfo)) {
        return false;
    }

    if (!CheckFileServers(taskInfo)) {
        return false;
    }
    return true;
}

static inline bool CheckRole(mp_uint32 role)
{
    return (role == static_cast<mp_uint32>(DwsRepoRole::MASTER) ||
            role == static_cast<mp_uint32>(DwsRepoRole::SLAVE));
}

template<typename T>
static inline bool CheckMasterRoleCount(const T &info)
{
    mp_uint32 masterRepoCount = 0;
    for (const auto &iter : info) {
        if (iter.role == static_cast<mp_uint32>(DwsRepoRole::MASTER)) {
            masterRepoCount++;
        }
    }
    if (masterRepoCount != 1) {
        ERRLOG("More than one master repository.masterRepoCount=%d", masterRepoCount);
        return false;
    }
    return true;
}

bool DwsTaskInfoParser::CheckRepositories(const DwsTaskInfo &taskInfo)
{
    if (taskInfo.copyType == CopyDataType::type::CLOUD_STORAGE_COPY) {
        return true;
    }

    if (taskInfo.repositories.empty()) {
        ERRLOG("Repositories is empty.");
        return false;
    }

    for (const auto &iter : taskInfo.repositories) {
        if (!CheckRole(iter.role)) {
            ERRLOG("Invalid repo role=%d", iter.role);
            return false;
        }

        if (iter.deviceSN.empty()) {
            ERRLOG("Esn is empty.");
            return false;
        }

        if (!CheckFilesystems(iter.filesystems)) {
            return false;
        }
    }

    if (!CheckMasterRoleCount(taskInfo.repositories)) {
        ERRLOG("Master role count invalid.");
        return false;
    }

    return true;
}

bool DwsTaskInfoParser::CheckFilesystems(const std::vector<DwsFsInfo> &filesystems)
{
    if (filesystems.empty()) {
        ERRLOG("Filesystems is empty.");
        return false;
    }

    for (const auto &iter : filesystems) {
        if (iter.id.empty()) {
            ERRLOG("Filesystem id is empty.");
            return false;
        }
        if (iter.name.empty()) {
            ERRLOG("Filesystem name is empty.");
            return false;
        }
        if (iter.sharePath.empty()) {
            ERRLOG("Filesystem sharePath is empty.");
            return false;
        }
        if (iter.mountPath.empty()) {
            ERRLOG("Filesystem mountPath is empty.");
            return false;
        }
    }

    return true;
}

static inline bool CheckPort(mp_uint32 port)
{
    const mp_uint32 maxPort = 65535;
    return (port > 0 && port <= maxPort);
}

bool DwsTaskInfoParser::CheckFileServers(const DwsTaskInfo &taskInfo)
{
    if (taskInfo.copyType != CopyDataType::type::CLOUD_STORAGE_COPY) {
        return true;
    }
    if (taskInfo.fileServers.empty()) {
        ERRLOG("No fileServers in taskInfo.");
        return false;
    }
    for (const auto &iter : taskInfo.fileServers) {
        if (iter.ip.empty()) {
            ERRLOG("Fileserver ip is empty");
            return false;
        }
        if (!CheckPort(iter.port)) {
            ERRLOG("Fileserver port is invalid.port=%u.", iter.port);
            return false;
        }
    }
    return true;
}

mp_int32 DwsTaskInfoParser::ParseFsRelation(const mp_string &filePath, DwsFsRelation &relation)
{
    mp_string readLine;
    mp_int32 ret = ReadFile(filePath, readLine);
    if (ret != MP_SUCCESS) {
        ERRLOG("Read %s failed!", filePath.c_str());
        return MP_FAILED;
    }

    if (!JsonHelper::JsonStringToStruct(readLine, relation)) {
        ERRLOG("Parse %s failed!readLine=%s.", filePath.c_str(), readLine.c_str());
        return MP_FAILED;
    }

    INFOLOG("FilePath=%s, readLine=%s.", filePath.c_str(), readLine.c_str());
    if (!CheckFsRelations(relation)) {
        ERRLOG("Check filesystem relation failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 DwsTaskInfoParser::ParseBusConfig(const mp_string &cachePath,
                                           const mp_string &copyId,
                                           XbsaBusinessConfig &busConfig)
{
    mp_string readLine;
    const mp_string busConfigPath = cachePath + "/tmp/" + copyId + "/business_config.txt";
    mp_int32 ret = ReadFile(busConfigPath, readLine);
    if (ret != MP_SUCCESS) {
        ERRLOG("Read %s failed!", busConfigPath.c_str());
        return MP_FAILED;
    }

    if (!JsonHelper::JsonStringToStruct(readLine, busConfig)) {
        ERRLOG("Parse %s failed!readLine=%s.", busConfigPath.c_str(), readLine.c_str());
        return MP_FAILED;
    }

    if (!CheckBusConfig(busConfig)) {
        ERRLOG("Check business config failed!readLine=%s.", readLine.c_str());
        return MP_FAILED;
    }

    INFOLOG("Parse business config success!jobType=%d.", busConfig.jobType);
    return MP_SUCCESS;
}

bool DwsTaskInfoParser::CheckBusConfig(const XbsaBusinessConfig &busConfig)
{
    if ((busConfig.jobType != static_cast<mp_uint32>(XbsaJobType::FULL_BACKUP)) &&
        (busConfig.jobType != static_cast<mp_uint32>(XbsaJobType::DIFF_BACKUP)) &&
        (busConfig.jobType != static_cast<mp_uint32>(XbsaJobType::FULL_RESTORE)) &&
        (busConfig.jobType != static_cast<mp_uint32>(XbsaJobType::DIFF_RESTORE))) {
        return false;
    }
    return true;
}

bool DwsTaskInfoParser::CheckFsRelations(const DwsFsRelation &relation)
{
    if (relation.relations.empty()) {
        ERRLOG("Filesystem relation map is empty.");
        return MP_FAILED;
    }

    for (const auto &iter : relation.relations) {
        if (!CheckRole(iter.role)) {
            ERRLOG("Invalid repo role=%d", iter.role);
            return false;
        }

        if (iter.oldEsn.empty() || iter.oldFsId.empty() || iter.oldFsName.empty()) {
            ERRLOG("oldEsn=%s or oldFsId=%s or oldFsName=%s is empty.",
                iter.oldEsn.c_str(), iter.oldFsId.c_str(), iter.oldFsName.c_str());
            return false;
        }

        if (iter.newEsn.empty() || iter.newFsId.empty() || iter.newFsName.empty()) {
            ERRLOG("newEsn=%s or newFsId=%s or newFsName=%s is empty.",
                iter.newEsn.c_str(), iter.newFsId.c_str(), iter.newFsName.c_str());
            return false;
        }
    }

    return true;
}

EXTER_ATTACK mp_int32 DwsTaskInfoParser::ReadFile(const mp_string &filePath, mp_string &readLine)
{
    struct stat buf;
    mp_int32 ret = lstat(filePath.c_str(), &buf);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get file stat failed! filePath(%s),errno=%d", filePath.c_str(), errno);
        return MP_FAILED;
    }
    if (S_ISLNK(buf.st_mode)) { // safe check:link file not permitted.
        ERRLOG("File is link file! filePath(%s)", filePath.c_str());
        return MP_FAILED;
    }

    mp_int32 maxSize = 0;
    ret = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_DWS_TMP_FILE_MAX_SIZE, maxSize);
    if (ret != MP_SUCCESS || maxSize <= 0 || maxSize > (INT_MAX / KB_TO_BYTE)) {
        ERRLOG("Get %s.%s cfg fail!maxSize=%d", CFG_BACKUP_SECTION.c_str(), CFG_DWS_TMP_FILE_MAX_SIZE.c_str(), maxSize);
        return MP_FAILED;
    }

    maxSize *= KB_TO_BYTE;
    if (buf.st_size > maxSize) { // safe check:file size should be in a proper range.
        ERRLOG("File size invalid!fileSize=%u,maxSize=%d", buf.st_size, maxSize);
        return MP_FAILED;
    }

    std::ifstream stream(filePath);
    if (!stream.is_open()) {
        ERRLOG("Open file failed.filePath=%s,errno=%d", filePath.c_str(), errno);
        return MP_FAILED;
    }
    std::getline(stream, readLine);
    if (readLine.empty()) {
        ERRLOG("Read file failed.filePath=%s,errno=%d", filePath.c_str(), errno);
        return MP_FAILED;
    }
    DBGLOG("Read file success.filePath=%s,readLine:%s", filePath.c_str(), readLine.c_str());
    return MP_SUCCESS;
}