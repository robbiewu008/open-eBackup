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
#ifndef SQLITE_DB_OPS_H
#define SQLITE_DB_OPS_H

#include <list>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <queue>
#include <memory>
#include <mutex>

#include "sqlite3.h"
#include "log/Log.h"
#include "Snowflake.h"

namespace Sqlite {
// sqlite data base table Name
const std::string SQLLITE_INDEX_TABLE  = "T_COPY_METADATA";   // sqlite index table

// Filed Names of T_COPY_METADATA
const std::string SQLITE_INDEX_TABLE_UUID          = "UUID";
const std::string SQLITE_INDEX_TABLE_TYPE          = "TYPE";
const std::string SQLITE_INDEX_TABLE_FILENAME      = "NAME";
const std::string SQLITE_INDEX_TABLE_PARENT_PATH   = "PARENT_PATH";
const std::string SQLITE_INDEX_TABLE_PARENT_UUID   = "PARENT_UUID";
const std::string SQLITE_INDEX_TABLE_FILESIZE      = "SIZE";
const std::string SQLITE_INDEX_TABLE_CREATE_TIME   = "CREATE_TIME";
const std::string SQLITE_INDEX_TABLE_MODIFY_TIME   = "MODIFY_TIME";
const std::string SQLITE_INDEX_TABLE_EXTEND_INFO   = "EXTEND_INFO";
const std::string SQLITE_INDEX_TABLE_RES_TYPE      = "RES_TYPE";
const std::string SQLITE_INDEX_TABLE_RES_SUB_TYPE  = "RES_SUB_TYPE";
#ifdef _OBS
const std::string SQLITE_INDEX_TABLE_META_INFO     = "META_INFO";
#endif

#ifdef _OBS
const std::string CREATETABLE =
"CREATE TABLE IF NOT EXISTS T_COPY_METADATA("  \
      "UUID             VARCHAR(512)    PRIMARY KEY    NOT NULL," \
      "NAME             VARCHAR(255)    NOT NULL    UNIQUE," \
      "TYPE             VARCHAR(1)      NOT NULL," \
      "PARENT_PATH      VARCHAR(32)     NOT NULL," \
      "PARENT_UUID      VARCHAR(512)," \
      "SIZE             BIGINT," \
      "CREATE_TIME      VARCHAR(64)," \
      "MODIFY_TIME      VARCHAR(64)," \
      "EXTEND_INFO      VARCHAR(64)," \
      "RES_TYPE         VARCHAR(1024)   NOT NULL    UNIQUE," \
      "RES_SUB_TYPE     VARCHAR(8)," \
      "META_INFO        TEXT);";
#else
const std::string CREATETABLE =
"CREATE TABLE IF NOT EXISTS T_COPY_METADATA(" \
      "UUID             VARCHAR(512)    PRIMARY KEY    NOT NULL," \
      "NAME             VARCHAR(255)    NOT NULL    UNIQUE," \
      "TYPE             VARCHAR(1)      NOT NULL," \
      "PARENT_PATH      VARCHAR(32)     NOT NULL," \
      "PARENT_UUID      VARCHAR(512)," \
      "SIZE             BIGINT,"      \
      "CREATE_TIME      VARCHAR(64)," \
      "MODIFY_TIME      VARCHAR(64)," \
      "EXTEND_INFO      VARCHAR(64)," \
      "RES_TYPE         VARCHAR(8),"  \
      "RES_SUB_TYPE     VARCHAR(8));";
#endif

const int32_t SQL_RETRY_MAX = 50;
const int32_t SQL_SLEEP_TIME_MIN = 100;
const int32_t SQL_SLEEP_TIME_MAX = 500;

const std::string STR_COMMA  =    "," ;
const std::string STR_OPEN_BRACE  =  "(" ;
const std::string STR_CLOSE_BRACE  =  ")" ;

const char CHAR_COMMA  =    ',' ;
} // namespace Sqlite

#define ATOINT32(x)  uint32_t(atoi(x))
#define ATOINT64(x)  uint64_t(atoll(x))
#define INT_TO_STRING(i, s)        \
    do {                         \
        s = std::to_string(i);                \
    } while (0);

typedef enum {
    DB_PARAM_TYPE_INT32,
    DB_PARAM_TYPE_INT64,
    DB_PARAM_TYPE_UINT32,
    DB_PARAM_TYPE_UINT64,
    DB_PARAM_TYPE_STRING
} DbParamType;

class  DbParam {
public:
    DbParamType m_type;
    std::string m_value;
    DbParam()
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = "";
    }
    DbParam(std::string value)
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = value;
    }
    DbParam(int32_t value);
    DbParam(int64_t value);
    DbParam(uint32_t value);
    DbParam(uint64_t value);
};

// Input parameter used for query in precompilation mode

class  DbParamStream {
public:
    DbParamStream()
    {}
    ~DbParamStream()
    {}
    void Clear()
    {
        m_ParamList.clear();
    }
    bool Empty() const
    {
        return m_ParamList.empty();
    }
    DbParam operator>>(DbParam& param);
    DbParam operator<<(DbParam& param)
    {
        m_ParamList.push_back(param);
        return param;
    }

    DbParamStream& operator<<(const DbParam&& param)
    {
        m_ParamList.push_back(param);
        return *this;
    }

    DbParamStream& operator<<(DbParam&& param)
    {
        m_ParamList.push_back(param);
        return *this;
    }

private:
    std::list<DbParam> m_ParamList;
};

class  DBReader {
public:
    DBReader();
    ~DBReader();
    std::string operator>>(std::string& strResult);
    std::string operator<<(std::string& strResult);
    void Clear();
    bool Empty() const;

private:
    std::list<std::string> m_lstResult;
};

class SQLiteCoreInfo {
public:
    explicit SQLiteCoreInfo(sqlite3* db, const std::string &dbFile);
    ~SQLiteCoreInfo();

    sqlite3_stmt* SqlPrepare(const std::string& sql) const;
    int32_t SqlBind(sqlite3_stmt* stmt, DbParamStream& dps);
    int32_t SqlStepExecute(sqlite3_stmt* stmt);
    int32_t FinalizeSqlStmt(sqlite3_stmt* stmt) const;
    int32_t SqlQuery(sqlite3_stmt* stmt, DBReader& readBuff, int32_t& rowCountParam, int32_t& colCountParam);

    // begins the sql txn statement
    int32_t BeginTrans() const;
    // rollback the sql txn statement
    int32_t RollbackTrans() const;
    // commit the sql txn
    int32_t CommitTrans() const;
    // sqlite3 object is successfully destroyed and all associated resources are deallocated.
    int32_t Disconnect();

    sqlite3* m_pDB { nullptr };
    std::string m_dbFile;
    std::list<std::string> m_stringList;
    std::mutex m_sqliteCoreInfoMutex;
};

class QueryTableArgs {
public:
    std::string dbFile;
    std::string sql;
    int32_t rowCount = 0;
    int32_t colCount = 0;
    DBReader readBuff;
};

class AggSqlRestoreInfo {
public:
    std::string normalFileName;
    uint64_t normalFileOffset;
    uint64_t normalFileSize;
};

class AggSqlRestoreQueryInfo {
public:
    std::string blobFileName ;
    uint64_t blobFileSize ;
    uint64_t fileOffset;
    uint64_t normalFileSize;
};

class SQLiteDB {
public:
    // metaRepoPath is local mount path + /flr/index/
    explicit SQLiteDB(const std::string& metaRepoPath, const std::string& metaAliasPath,
        std::shared_ptr<Module::Snowflake> idGenerator)
    {
        m_metaPath = metaRepoPath;
        m_metaAliasPath = metaAliasPath;
        m_idGenerator = idGenerator;
    }
    ~SQLiteDB();

    // prepares the db and table. dbFile is the fulldirpath of nas share including dbfilename
    std::shared_ptr<SQLiteCoreInfo> PrepareDb(const std::string& dbFile, int flags);
    int32_t DeleteAndPrepareDb(const std::string& dbFile, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr, int flags);

    int32_t PrepareSqlStmt(const std::string& dbFile, sqlite3_stmt** const sqlStmt,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const;
    int32_t BindStepSql(sqlite3_stmt* const sqlStmt, DbParamStream& dps,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const;
    int32_t FinalizeSqlStmt(sqlite3_stmt* const sqlStmt, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const;
    // executes the sql query statements
    int32_t QueryTable(QueryTableArgs &queryTableArgs, DbParamStream& dps,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const;
    // sqlite3 object is successfully destroyed and all associated resources are deallocated.
    int32_t Disconnect(const std::string& dbFile, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr);
    std::shared_ptr<SQLiteCoreInfo> Connect(const std::string& dbFile, uint32_t flags, bool isReconnect = false,
        std::shared_ptr<SQLiteCoreInfo> currSqlInfoPtr = nullptr);
    void AddDbToMap(const std::string& dbFile, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr);
    std::shared_ptr<SQLiteCoreInfo> QueryDbFromMap(const std::string& dbFile);
    void DeleteAllDbFromMap();
    void CheckAndDeleteAllDbFromMap();
    void DeleteFromMap(const std::string& dbFile);
    std::string GetMetaPath();
    std::string ConcatDbFullPath(const std::string& dbFileName) const;
    bool GetAliasForDbPath(std::string& dbFullPath) const;
    int32_t CreateTable(sqlite3* pDB);

private:
    std::shared_ptr<Module::Snowflake> m_idGenerator;
    std::string m_metaPath;     // this is prefix path of the all the db files i.e. The local mount path
    std::string m_metaAliasPath;
    std::mutex m_sqliteDbMtx;
    std::mutex m_dbMapMtx;
    std::map<std::string, std::shared_ptr<SQLiteCoreInfo>> m_dBMap;
};

class IndexDetails {
public:
    IndexDetails()
    {
    }
    std::string m_uuid {};
    std::string m_actualFileName {};
    std::string m_type {};
    uint64_t m_actualFileSize = 0;
    std::string m_metaFileName {};
    uint64_t m_offset = 0;
    std::string m_aggregateFileName {};  // SQLITE_INDEX_TABLE_PARENT_PATH
    uint64_t m_aggregatedFileSize = 0;
    std::string m_parentUUID {};
    uint64_t m_createTime {};
    uint64_t m_modifyTime {};
    std::string m_resType {}; // Object storage backup uses this field to save key.
    std::string m_resSubType {};
    std::string m_metaData {};

    void Uint64ToString(const uint64_t integerVal, std::string& strVal) const;
    void StringToUint64(const std::string strVal, uint64_t& integerVal) const;
    // Fill Index detail params by query table
    void FillIndexDetailsParams(QueryTableArgs &queryTableArgs, IndexDetails &indexInfo);

    int32_t InsertIndexInfo(std::shared_ptr<SQLiteDB> sqliteDB, sqlite3_stmt* const sqlStmt,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr);

    int32_t DeleteIndexInfoByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
        const std::string& normalFileName);

    int64_t QueryRecordNum(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile, int64_t& count);

    // Query all the index info from db
    int32_t QueryAllIndexInfo(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
        std::vector<IndexDetails>& vecIndexInfo, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore);
    // Query Index Info by Name from db
    int32_t QueryIndexInfoByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
        std::vector<IndexDetails>& vecIndexInfo, const std::string& normalFileName, const std::string type);

    int32_t QueryZipNameNSizeByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
        const std::string& normalFileName, AggSqlRestoreQueryInfo &info,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore);

    int32_t QueryNormalFilesByAggName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
        std::vector<AggSqlRestoreInfo>& vecNormalFilesList, const std::string& aggFileName,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore);

    int32_t QueryNormalFilesByName(std::shared_ptr<SQLiteDB> sqliteDB, std::string& fileName,
        std::string& dbFile, AggSqlRestoreQueryInfo &info, std::vector<AggSqlRestoreInfo> &vecNormalFiles);

    int32_t MergeDbFile(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dstDbFile, const std::string& srcDbFile);
};

#endif // SQLITE_DB_OPS_H