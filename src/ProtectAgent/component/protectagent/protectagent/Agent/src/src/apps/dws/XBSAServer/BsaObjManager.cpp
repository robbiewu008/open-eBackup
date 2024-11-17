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
#include "apps/dws/XBSAServer/BsaObjManager.h"
#include <common/CMpTime.h>
#include "common/Log.h"
#include "common/Utils.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaDb.h"

mp_int32 BsaObjManager::CreateObject(BsaObjInfo &objInfo)
{
    mp_time t;
    CMpTime::Now(t);
    objInfo.timestamp = std::to_string(t);
    objInfo.restoreOrder = 0;
    objInfo.objectStatus = BSA_ObjectStatus_MOST_RECENT;
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 BsaObjManager::QueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
    std::vector<BsaObjInfo> &result, mp_long bsaHandle)
{
    int32_t appType = BsaSessionManager::GetInstance().GetSessionAppType(bsaHandle);
    switch (appType) {
        case BSA_AppType::BSA_DWS: {
            return DwsQueryObject(queryCond, pageInfo, result, bsaHandle);
        }
        case BSA_AppType::BSA_INFORMIX: {
            return InformixQueryObject(queryCond, pageInfo, result, bsaHandle);
        }
        case BSA_AppType::BSA_HCS: {
            return HcsQueryObject(queryCond, pageInfo, result, bsaHandle);
        }
        case BSA_AppType::BSA_TPOPS: {
            return HcsQueryObject(queryCond, pageInfo, result, bsaHandle);
        }
        default: {
            ERRLOG("Invalid apptype %d", appType);
            return MP_FAILED;
        }
    }
}

mp_int32 BsaObjManager::InformixQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
    std::vector<BsaObjInfo> &result, mp_long bsaHandle)
{
    mp_string objectName = queryCond.objectName;
    INFOLOG("objectSpaceName: %s, objectName: %s.", queryCond.objectSpaceName.c_str(), queryCond.objectName.c_str());
    std::vector<mp_string> vecTokens;
    CMpString::StrSplit(vecTokens, objectName, '/');
    if (vecTokens.size() < AWK_COL_FIRST_2) {
        ERRLOG("The format is incorrect, objectName: %s.", objectName.c_str());
        return MP_FAILED;
    }
    mp_string instanceName = vecTokens[AWK_COL_FIRST_1];
    DwsCacheInfo cacheInfo = BsaSessionManager::GetInstance().GetSessionCacheInfo(bsaHandle);
    mp_string dbFile = cacheInfo.cacheRepoPath + "/meta/" + cacheInfo.copyId + "/objectmeta/backupkey.db";
    INFOLOG("cacheRepoPath: %s, Db file path:%s", cacheInfo.cacheRepoPath.c_str(), dbFile.c_str());
    if (dbFile.empty()) {
        ERRLOG("Get db file path failed!");
        return MP_FAILED;
    }

    if (!CMpFile::FileExist(dbFile)) {
        ERRLOG("Db file not exist!dbFile=%s.errno=%d.", dbFile.c_str(), errno);
        return MP_FAILED;
    }
    BsaDb db(dbFile);
    return db.QueryBsaObjs(queryCond, pageInfo, result);
}

mp_int32 BsaObjManager::DwsQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
                                       std::vector<BsaObjInfo> &result, mp_long bsaHandle)
{
    DwsCacheInfo cacheInfo = BsaSessionManager::GetInstance().GetSessionCacheInfo(bsaHandle);
    mp_string dbFile = cacheInfo.cacheRepoPath + "/meta/" + cacheInfo.copyId + "/objectmeta/backupkey.db";
    DBGLOG("Db file path:%s", dbFile.c_str());
    if (dbFile.empty()) {
        ERRLOG("Get db file path failed!");
        return MP_FAILED;
    }

    if (!CMpFile::FileExist(dbFile)) {
        ERRLOG("Db file not exist!dbFile=%s.errno=%d.", dbFile.c_str(), errno);
        return MP_FAILED;
    }

    BsaDb db(dbFile);
    return db.QueryBsaObjs(queryCond, pageInfo, result);
}

mp_int32 BsaObjManager::HcsQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
                                       std::vector<BsaObjInfo> &result, mp_long bsaHandle)
{
    DwsTaskInfoParser parser;
    XbsaBusinessConfig busConfig;
    DwsCacheInfo cacheInfo = BsaSessionManager::GetInstance().GetSessionCacheInfo(bsaHandle);
    if (parser.ParseBusConfig(cacheInfo.cacheRepoPath, cacheInfo.copyId, busConfig) != MP_SUCCESS) {
        ERRLOG("Fail to read business config.Default value: full restore.");
        return MP_FAILED;
    }
    if (busConfig.jobType != static_cast<mp_uint32>(XbsaJobType::FULL_BACKUP)) {
        mp_string dbFile = cacheInfo.cacheRepoPath + "/meta/" + cacheInfo.copyId + "/objectmeta/backupkey.db";
        DBGLOG("Db file path:%s", dbFile.c_str());
        if (dbFile.empty()) {
            ERRLOG("Get db file path failed!");
            return MP_FAILED;
        }

        if (!CMpFile::FileExist(dbFile)) {
            ERRLOG("Db file not exist!dbFile=%s.errno=%d.", dbFile.c_str(), errno);
            return MP_FAILED;
        }

        BsaDb db(dbFile);
        return db.QueryBsaObjs(queryCond, pageInfo, result);
    }

    std::vector<mp_string> subdirs;
    if (CMpFile::GetFolderDir(cacheInfo.metaRepoPath, subdirs) != MP_SUCCESS) {
        ERRLOG("There is no objectmeta subdir in %s", cacheInfo.metaRepoPath.c_str());
        return MP_FAILED;
    }
    for (int i = 0; i < subdirs.size(); ++i) {
        std::vector<mp_string> temp_file_name;
        mp_string temp_subdir_path = cacheInfo.metaRepoPath + subdirs[i];
        DBGLOG("Subdir path: %s", temp_subdir_path.c_str());
        CMpFile::GetFolderFile(temp_subdir_path, temp_file_name);
        for (auto &file_name : temp_file_name) {
            mp_string full_db_name = cacheInfo.metaRepoPath + subdirs[i] + "/" + file_name;
            DBGLOG("Full db name: %s", full_db_name.c_str());
            BsaDb db(full_db_name);
            std::vector<BsaObjInfo> tmp_obj_info;
            if (db.QueryBsaObjs(queryCond, pageInfo, tmp_obj_info) != MP_SUCCESS) {
                WARNLOG("Fail to query from %s", full_db_name.c_str());
                return MP_FAILED;
            }
            result.insert(result.end(), tmp_obj_info.begin(), tmp_obj_info.end());
        }
    }
    return MP_SUCCESS;
}

mp_int32 BsaObjManager::SaveObjects(const std::map<mp_uint64, BsaObjInfo> &objList, mp_long bsaHandle)
{
    mp_string dbFile = BsaSessionManager::GetInstance().GetBsaDbFilePath(bsaHandle);
    if (dbFile.empty()) {
        ERRLOG("Get db file path failed!");
        return MP_FAILED;
    }

    if (!CMpFile::FileExist(dbFile)) {
        ERRLOG("Db file not exist!dbFile=%s.errno=%d.", dbFile.c_str(), errno);
        return MP_FAILED;
    }

    INFOLOG("dbFile: %s", dbFile.c_str());

    std::lock_guard<std::mutex> lock(m_mutexTrans);
    BsaDb db(dbFile);

    for (const auto &iter : objList) {
        if (db.InsertBsaObj(iter.second) != MP_SUCCESS) {
            ERRLOG("Insert db failed!objName=%s.", iter.second.objectName.c_str());
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}