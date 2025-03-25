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
#include "apps/dws/XBSAServer/BsaDb.h"
#include "common/DB.h"
#include "common/Log.h"
#include "xbsa/xbsa.h"

using namespace std;

mp_int32 BsaDb::CreateBsaObjTable()
{
    DbParamStream dps;
    // 进程重启任务续作时，此时db文件可能已存在
    mp_string sql = "CREATE TABLE IF NOT EXISTS [BsaObjTable] ("
        "[copyId] VARCHAR(100) NOT NULL,"
        "[objectSpaceName] VARCHAR(1024),"
        "[objectName] VARCHAR(1024) NOT NULL,"
        "[bsaObjectOwner] VARCHAR(64),"
        "[appObjectOwner] VARCHAR(64),"
        "[copyType] INTEGER(8),"
        "[estimatedSize] VARCHAR(100) NOT NULL,"
        "[resourceType] VARCHAR(32),"
        "[objectType] INTEGER(8),"
        "[objectStatus] INTEGER(8),"
        "[objectDescription] VARCHAR(100),"
        "[objectInfo] VARCHAR(256),"
        "[timestamp] VARCHAR(64),"
        "[restoreOrder] VARCHAR(100),"
        "[storePath] VARCHAR(1280) NOT NULL,"
        "[filesystemName] VARCHAR(256) NOT NULL,"
        "[filesystemId] VARCHAR(128) NOT NULL,"
        "[filesystemDeviceId] VARCHAR(256) NOT NULL,"
        "[rsv1] VARCHAR(256),"
        "[rsv2] VARCHAR(256));";

    mp_int32 ret = DWSDB::GetInstance().ExecSql(m_dbFile, sql, dps);
    if (ret != MP_SUCCESS) {
        ERRLOG("ExecSql failed, ret=%d,sql(%s).dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
    }

    return ret;
}

mp_int32 BsaDb::InsertBsaObj(const BsaObjInfo &obj)
{
    mp_string sql = "insert into BsaObjTable"
        "(copyId,objectSpaceName,objectName,bsaObjectOwner,appObjectOwner,copyType,estimatedSize,resourceType,"
        "objectType,objectStatus,objectDescription,objectInfo,timestamp,restoreOrder,storePath,filesystemName,"
        "filesystemId,filesystemDeviceId)values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    DbParamStream dps;
    DbParam dp = std::to_string(obj.copyId);
    dps << std::move(dp);
    dp = obj.objectSpaceName;
    dps << std::move(dp);
    dp = obj.objectName;
    dps << std::move(dp);
    dp = obj.bsaObjectOwner;
    dps << std::move(dp);
    dp = obj.appObjectOwner;
    dps << std::move(dp);
    dp = obj.copyType;
    dps << std::move(dp);
    dp = std::to_string(obj.estimatedSize);
    dps << std::move(dp);
    dp = obj.resourceType;
    dps << std::move(dp);
    dp = obj.objectType;
    dps << std::move(dp);
    dp = obj.objectStatus;
    dps << std::move(dp);
    dp = obj.objectDescription;
    dps << std::move(dp);
    dp = obj.objectInfo;
    dps << std::move(dp);
    dp = obj.timestamp;
    dps << std::move(dp);
    dp = std::to_string(obj.restoreOrder);
    dps << std::move(dp);
    dp = obj.storePath;
    dps << std::move(dp);
    dp = obj.fsName;
    dps << std::move(dp);
    dp = obj.fsId;
    dps << std::move(dp);
    dp = obj.fsDeviceId;
    dps << std::move(dp);

    mp_int32 ret = DWSDB::GetInstance().ExecSql(m_dbFile, sql, dps);
    if (ret != MP_SUCCESS) {
        ERRLOG("ExecSql failed, ret=%d,sql(%s).dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
    }

    return MP_SUCCESS;
}

mp_void BsaDb::TransObjParam(DBReader &readBuff, BsaObjInfo &obj)
{
    mp_string strId;
    readBuff >> strId;
    CMpString::StringToUInteger(strId, obj.copyId);
    readBuff >> obj.objectSpaceName;
    readBuff >> obj.objectName;
    readBuff >> obj.bsaObjectOwner;
    readBuff >> obj.appObjectOwner;
    strId = "";
    readBuff >> strId;
    obj.copyType = std::stoul(strId);
    strId = "";
    readBuff >> strId;
    CMpString::StringToUInteger(strId, obj.estimatedSize);
    readBuff >> obj.resourceType;
    strId = "";
    readBuff >> strId;
    obj.objectType = std::stoul(strId);
    strId = "";
    readBuff >> strId;
    obj.objectStatus = std::stoul(strId);
    readBuff >> obj.objectDescription;
    readBuff >> obj.objectInfo;
    readBuff >> obj.timestamp;
    strId = "";
    readBuff >> strId;
    CMpString::StringToUInteger(strId, obj.restoreOrder);
    readBuff >> obj.storePath;
    readBuff >> obj.fsName;
    readBuff >> obj.fsId;
    readBuff >> obj.fsDeviceId;
}

mp_int32 BsaDb::QueryObject(const mp_string &sql, DbParamStream &dps, std::vector<BsaObjInfo> &objList)
{
    mp_int32 rowCt = 0;
    mp_int32 colCnt = 0;
    DBReader readBuff;

    objList.clear();

    mp_int32 ret = DWSDB::GetInstance().QueryTable(m_dbFile, sql, dps, readBuff, rowCt, colCnt);
    if (ret != MP_SUCCESS) {
        ERRLOG("QueryTable failed, ret=%d,sql(%s),dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
        return ret;
    }

    try {
        objList.reserve(rowCt);
        for (mp_int32 i = 0; i < rowCt; i++) {
            BsaObjInfo obj;
            TransObjParam(readBuff, obj);
            objList.emplace_back(obj);
        }
    } catch (const std::exception &e) {
        ERRLOG("Parse query result failed, sql(%s), exception:%s.", sql.c_str(), e.what());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_void BsaDb::BuildQueryCond(const BsaObjInfo &queryCond, mp_string &sql, DbParamStream &dps)
{
    if (queryCond.appObjectOwner != "*" && !queryCond.appObjectOwner.empty()) {
        sql += " appObjectOwner == ? and ";
        DbParam dp = queryCond.appObjectOwner;
        dps << std::move(dp);
    }
    if (queryCond.bsaObjectOwner != "*" && !queryCond.bsaObjectOwner.empty()) {
        sql += " bsaObjectOwner == ? and ";
        DbParam dp = queryCond.bsaObjectOwner;
        dps << std::move(dp);
    }
    if (queryCond.objectSpaceName != "*" && !queryCond.objectSpaceName.empty()) {
        sql += " objectSpaceName == ? and ";
        DbParam dp = queryCond.objectSpaceName;
        dps << std::move(dp);
    }

    sql += " objectName like \'" + queryCond.objectName + "%\' ";

    if (queryCond.copyType != BSA_CopyType_ANY) {
        sql += "and copyType == ? ";
        DbParam dp = queryCond.copyType;
        dps << std::move(dp);
    }

    if (queryCond.objectType != BSA_ObjectType_ANY) {
        sql += " and objectType == ? ";
        DbParam dp = queryCond.objectType;
        dps << std::move(dp);
    }

    if (queryCond.objectStatus != BSA_ObjectStatus_ANY) {
        sql += "and objectStatus == ? ";
        DbParam dp = queryCond.objectStatus;
        dps << std::move(dp);
    }
}

mp_int32 BsaDb::QueryBsaObjs(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
    std::vector<BsaObjInfo> &objList)
{
    DbParamStream dps;
    mp_string sql = "select copyId,objectSpaceName,objectName,bsaObjectOwner,appObjectOwner,copyType,"
        "estimatedSize,resourceType,objectType,objectStatus,objectDescription,objectInfo,timestamp,"
        "restoreOrder,storePath,filesystemName,filesystemId,filesystemDeviceId from BsaObjTable where ";

    BuildQueryCond(queryCond, sql, dps);

    if (pageInfo.limit > 0) {
        sql += "limit ? offset ?;";
        DbParam dp = pageInfo.limit;
        dps << std::move(dp);
        dp = pageInfo.offset;
        dps << std::move(dp);
    } else {
        sql += ";";
    }

    DBGLOG("Sql:%s.limit=%d,offset=%d.", sql.c_str(), pageInfo.limit, pageInfo.offset);

    return QueryObject(sql, dps, objList);
}

mp_int32 BsaDb::BeginTrans()
{
    return DWSDB::GetInstance().BeginTrans(m_dbFile);
}

mp_int32 BsaDb::RollbackTrans()
{
    return DWSDB::GetInstance().RollbackTrans();
}

mp_int32 BsaDb::CommitTrans()
{
    return DWSDB::GetInstance().CommitTrans();
}

mp_int32 BsaDb::CreateDwsHostFilesystemTable()
{
    DbParamStream dps;
    mp_string sql = "CREATE TABLE IF NOT EXISTS [DwsHostFilesystemTable] ("
        "[hostname] VARCHAR(256) NOT NULL PRIMARY KEY,"
        "[filesystemName] VARCHAR(256) NOT NULL,"
        "[filesystemId] VARCHAR(128) NOT NULL,"
        "[filesystemDeviceId] VARCHAR(256) NOT NULL,"
        "[rsv1] VARCHAR(256));";

    mp_int32 ret = DWSDB::GetInstance().ExecSql(m_dbFile, sql, dps);
    if (ret != MP_SUCCESS) {
        ERRLOG("ExecSql failed, ret=%d,sql(%s).dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
    }

    return ret;
}

mp_int32 BsaDb::InsertDwsHost(const DwsHostInfo &host)
{
    mp_string sql = "insert into DwsHostFilesystemTable"
        "(hostname,filesystemName,filesystemId,filesystemDeviceId)values(?,?,?,?);";

    DbParamStream dps;
    DbParam dp = host.hostname;
    dps << std::move(dp);
    dp = host.fsName;
    dps << std::move(dp);
    dp = host.fsId;
    dps << std::move(dp);
    dp = host.fsDeviceId;
    dps << std::move(dp);

    mp_int32 ret = DWSDB::GetInstance().ExecSql(m_dbFile, sql, dps);
    if (ret != MP_SUCCESS) {
        ERRLOG("ExecSql failed, ret=%d,sql(%s).dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
    }

    return ret;
}

mp_int32 BsaDb::DeleteDwsHost(const mp_string &hostname)
{
    mp_string sql = "delete from DwsHostFilesystemTable where hostname == ?;";

    DbParamStream dps;
    DbParam dp = hostname;
    dps << std::move(dp);

    mp_int32 ret = DWSDB::GetInstance().ExecSql(m_dbFile, sql, dps);
    if (ret != MP_SUCCESS) {
        ERRLOG("ExecSql failed, ret=%d,sql(%s).dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
    }

    return ret;
}

mp_void BsaDb::TransHostParam(DBReader &readBuff, DwsHostInfo &host)
{
    readBuff >> host.hostname;
    readBuff >> host.fsName;
    readBuff >> host.fsId;
    readBuff >> host.fsDeviceId;
}

mp_int32 BsaDb::QueryDwsHosts(const BsaQueryPageInfo &pageInfo, std::vector<DwsHostInfo> &hostList)
{
    DbParamStream dps;
    mp_string sql = "select hostname,filesystemName,filesystemId,filesystemDeviceId from DwsHostFilesystemTable "
        "limit ? offset ?;";
    DbParam dp = pageInfo.limit;
    dps << std::move(dp);
    dp = pageInfo.offset;
    dps << std::move(dp);

    DBGLOG("Sql:limit=%d,offset=%d.", pageInfo.limit, pageInfo.offset);
    mp_int32 rowCt = 0;
    mp_int32 colCnt = 0;
    DBReader readBuff;

    mp_int32 ret = DWSDB::GetInstance().QueryTable(m_dbFile, sql, dps, readBuff, rowCt, colCnt);
    if (ret != MP_SUCCESS) {
        ERRLOG("QueryTable failed, ret=%d,sql(%s),dbFile(%s).", ret, sql.c_str(), m_dbFile.c_str());
        return ret;
    }

    try {
        hostList.reserve(rowCt);
        for (mp_int32 i = 0; i < rowCt; i++) {
            DwsHostInfo host;
            TransHostParam(readBuff, host);
            hostList.emplace_back(host);
        }
    } catch (const std::exception &e) {
        ERRLOG("Parse query result failed, sql(%s), exception:%s.", sql.c_str(), e.what());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}