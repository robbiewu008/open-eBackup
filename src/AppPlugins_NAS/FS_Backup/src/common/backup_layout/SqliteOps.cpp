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
#include "SqliteOps.h"
#include "io_device/FileSystemIO.hpp"
#include "Win32PathUtils.h"
#include "FSBackupUtils.h"
#include "common/Thread.h"

#ifdef WIN32
#include "Win32BackupEngineUtils.h"
#endif

using namespace std;
using namespace Sqlite;
using namespace FS_Backup;

namespace {
    const int32_t INDEX_META_FILE_NAME = 0;
    const int32_t INDEX_META_FILE_OFFSET = 1;
    const int32_t INDEX_AGGR_FILE_SIZE = 2;
    const int SUCCESS = 0;
    const int FAILED = 1;
    const int MAX_RETRY = 0;
    const int MAX_DB_OPEN_RETRY = 3;
    const int READ_DB_FLAGS = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
    // 重试时间设为120s， 保证3次重试大于5分钟， 解决nas share短时间断连问题
    const uint64_t RETRY_TIMEOUT = 120000;
    const uint32_t MAX_OPEN_DB_FILE_HANDLES = 32;
    const uint32_t MAX_PATH_LEN_SUPPORED_BY_SQLITE = 500;
    const std::string INDEX_DB_FILE_NAME = "copymetadata.sqlite";
    const std::string SLASH = "/";
    const std::string BACKSLASH = "\\";
}

void SleepForRandomMsec(uint32_t lower, uint32_t upper)
{
    uint32_t sleepTime = (rand() % (upper - lower + 1)) + lower;
    sqlite3_sleep(sleepTime);
}

int SQLBusyCallback(void* ptr, int retryNum)
{
    ptr = ptr;
    if (retryNum == SQL_RETRY_MAX) {
        /* Max retry is completed, return 0 to sqlite, so the sqilte api will return SQLITE_BUSY to app */
        ERRLOG("SQLITE_BUSY retryNum=%d", retryNum);
        return 0;
    }
    /* retrun 1 after random sleep to sqlite, so it will try attempt the OP again */
    WARNLOG("SQLITE_BUSY, retryNum=%d", retryNum);
    SleepForRandomMsec(SQL_SLEEP_TIME_MIN, SQL_SLEEP_TIME_MAX);
    return 1;
}

DBReader::DBReader()
{
    m_lstResult.clear();
}

DBReader::~DBReader()
{
    m_lstResult.clear();
}


void DBReader::Clear()
{
    m_lstResult.clear();
}


bool DBReader::Empty() const
{
    return m_lstResult.empty();
}

std::string DBReader::operator<<(std::string& strResult)
{
    m_lstResult.push_back(strResult);
    return strResult;
}


std::string DBReader::operator>>(std::string& strResult)
{
    list<std::string>::iterator ite = m_lstResult.begin();
    if (m_lstResult.end() != ite) {
        strResult = *ite;
        m_lstResult.erase(ite);
    } else {
        DBGLOG("There has no param.");
    }
    return strResult;
}

DbParam DbParamStream::operator>>(DbParam& param)
{
    if (Empty()) {
        DBGLOG("There has no param.");
        return param;
    }
    param = m_ParamList.front();
    m_ParamList.pop_front();
    return param;
}

DbParam::DbParam(int32_t value) :      m_type(DB_PARAM_TYPE_INT32)
{
    INT_TO_STRING(value, m_value);
}
DbParam::DbParam(int64_t value) :      m_type(DB_PARAM_TYPE_INT64)
{
    INT_TO_STRING(value, m_value);
}
DbParam::DbParam(uint32_t value) :      m_type(DB_PARAM_TYPE_UINT32)
{
    INT_TO_STRING(value, m_value);
}
DbParam::DbParam(uint64_t value) :      m_type(DB_PARAM_TYPE_UINT64)
{
    INT_TO_STRING(value, m_value);
}


SQLiteCoreInfo::SQLiteCoreInfo(sqlite3* db, const std::string &dbFile) : m_pDB(db), m_dbFile(dbFile)
{
}

SQLiteCoreInfo::~SQLiteCoreInfo()
{
    Disconnect();
}

sqlite3_stmt* SQLiteCoreInfo::SqlPrepare(const std::string& sql) const
{
    sqlite3_stmt* stmtPtr = nullptr;
    int32_t ret = SQLITE_OK;
    int32_t retryCnt = 0;
    do {
        ret = sqlite3_prepare_v2(m_pDB, sql.c_str(), sql.size(), &stmtPtr, nullptr);
        if (ret != SQLITE_OK && retryCnt < MAX_RETRY) {
            WARNLOG("sqlite3_prepare_v2 DB failed, ret=%d, %s, retrying: %d", ret, sqlite3_errmsg(m_pDB), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            retryCnt++;
        }
    } while (ret != SQLITE_OK && retryCnt < MAX_RETRY);

    if (ret != SQLITE_OK) {
        ERRLOG("sqlite3_prepare_v2 DB failed, ret=%d, %s", ret, sqlite3_errmsg(m_pDB));
        return nullptr;
    }
    return stmtPtr;
}

int32_t SQLiteCoreInfo::SqlBind(sqlite3_stmt* stmt, DbParamStream& dps)
{
    int32_t ret;
    if (!m_stringList.empty()) {
        m_stringList.clear();
    }

    for (int32_t i = 1; !dps.Empty(); i++) {
        DbParam dp;
        dps >> dp;

        switch (dp.m_type) {
            case DB_PARAM_TYPE_INT32:
                ret = sqlite3_bind_int(stmt, i, ATOINT32(dp.m_value.c_str()));
                break;
            case DB_PARAM_TYPE_UINT32:
            case DB_PARAM_TYPE_INT64:
            case DB_PARAM_TYPE_UINT64:
                ret = sqlite3_bind_int64(stmt, i, ATOINT64(dp.m_value.c_str()));
                break;
            case DB_PARAM_TYPE_STRING:
                m_stringList.push_front(dp.m_value);
                ret = sqlite3_bind_text(stmt, i, m_stringList.front().c_str(), m_stringList.front().size(), NULL);
                break;
            default:
                ret = SQLITE_ERROR;
        }

        if (ret != SQLITE_OK) {
            ERRLOG("sqlite3_bind_* DB failed, ret=%d, %s.", ret, sqlite3_errmsg(m_pDB));
            return FAILED;
        }
    }

    return SUCCESS;
}

int32_t SQLiteCoreInfo::SqlStepExecute(sqlite3_stmt* stmt)
{
    int32_t ret = SQLITE_DONE;
    int32_t retryCnt = 0;
    do {
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE && retryCnt < MAX_RETRY) {
            WARNLOG("sqlite3_step failed, ret=%d, %s, retrying: %d", ret, sqlite3_errmsg(m_pDB), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            retryCnt++;
        }
    } while (ret != SQLITE_DONE && retryCnt < MAX_RETRY);

    if (ret != SQLITE_DONE) {
        ERRLOG("sqlite3_step failed, ret=%d, %s", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }
    sqlite3_reset(stmt);
    if (!m_stringList.empty()) {
        m_stringList.clear();
    }
    return SUCCESS;
}

int32_t SQLiteCoreInfo::FinalizeSqlStmt(sqlite3_stmt* stmt) const
{
    int32_t ret = SQLITE_OK;
    int32_t retryCnt = 0;
    do {
        ret = sqlite3_finalize(stmt);
        if (ret != SQLITE_OK && retryCnt < MAX_RETRY) {
            WARNLOG("sqlite3_finalize failed, ret=%d, %s, retrying: %d", ret, sqlite3_errmsg(m_pDB), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            retryCnt++;
        }
    } while (ret != SQLITE_OK && retryCnt < MAX_RETRY);

    if (ret != SQLITE_OK) {
        ERRLOG("sqlite3_finalize failed, ret=%d, %s", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }

    if (retryCnt != 0) {
        WARNLOG("sqlite3_finalize succeeded after retry: %d", retryCnt);
    }

    return SUCCESS;
}

int32_t SQLiteCoreInfo::SqlQuery(sqlite3_stmt* stmt, DBReader& readBuff, int32_t& rowCountParam,
    int32_t& colCountParam)
{
    int32_t colCount = sqlite3_column_count(stmt);
    if (colCount <= 0) {
        ERRLOG("sqlite3_column_count failed");
        return FAILED;
    }

    int32_t rowCount = 0;
    int32_t ret = 0;
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int32_t i = 0; i < colCount; i++) {
            std::string text;
            const char* temp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            if (temp != nullptr) {
                text = temp;
            } else {
                ERRLOG("sqlite3_column_text null, col=%d.", i);
            }
            readBuff << text;
        }
        rowCount++;
    }

    if (ret != SQLITE_DONE) {
        ERRLOG("sqlite3_step not return SQLITE_DONE, ret=%d, %s.", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }

    rowCountParam = rowCount;
    colCountParam = colCount;
    if (!m_stringList.empty()) {
        m_stringList.clear();
    }
    return SUCCESS;
}

int32_t SQLiteCoreInfo::BeginTrans() const
{
    DBGLOG("Enter BeginTrans.");

    int32_t ret = SQLITE_OK;
    int32_t retryCnt = 0;
    do {
        ret = sqlite3_exec(m_pDB, "BEGIN", nullptr, nullptr, nullptr);
        if (ret != SQLITE_OK && retryCnt < MAX_RETRY) {
            WARNLOG("Begin transaction failed, ret=%d, %s, retrying: %d", ret, sqlite3_errmsg(m_pDB), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            retryCnt++;
        }
    } while (ret != SQLITE_OK && retryCnt < MAX_RETRY);

    if (ret != SQLITE_OK) {
        ERRLOG("Begin transaction failed, ret=%d, %s.", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }

    DBGLOG("Exit BeginTrans succ.");
    return SUCCESS;
}

int32_t SQLiteCoreInfo::CommitTrans() const
{
    DBGLOG("Enter CommitTrans.");
    int32_t ret = SQLITE_OK;
    int32_t retryCnt = 0;
    do {
        ret = sqlite3_exec(m_pDB, "COMMIT", nullptr, nullptr, nullptr);
        if (ret != SQLITE_OK && retryCnt < MAX_RETRY) {
            WARNLOG("Commit transaction failed, ret=%d, %s, retrying: %d", ret, sqlite3_errmsg(m_pDB), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            retryCnt++;
        }
    } while (ret != SQLITE_OK && retryCnt < MAX_RETRY);

    if (ret != SQLITE_OK) {
        ERRLOG("Commit transaction failed, ret=%d, %s.", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }

    DBGLOG("Exit CommitTrans succ.");
    return SUCCESS;
}

int32_t SQLiteCoreInfo::RollbackTrans() const
{
    DBGLOG("Enter RollbackTrans.");
    int32_t ret = sqlite3_exec(m_pDB, "ROLLBACK", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        ERRLOG("Rollback transaction failed, ret=%d, %s.", ret, sqlite3_errmsg(m_pDB));
        return FAILED;
    }

    DBGLOG("Exit RollbackTrans.");
    return SUCCESS;
}

int32_t SQLiteCoreInfo::Disconnect()
{
    if (m_pDB != nullptr) {
        DBGLOG("Closing dbFile: %s", m_dbFile.c_str());
        int32_t ret = sqlite3_close(m_pDB);
        if (ret != SQLITE_OK) {
            ERRLOG("sqlite3_close failed.errno: %d, %s, dbFile: %s", ret, sqlite3_errmsg(m_pDB), m_dbFile.c_str());
        }
        m_pDB = nullptr;
    }

    return SUCCESS;
}

SQLiteDB::~SQLiteDB()
{
    DBGLOG("SQLiteDB destructor");
    DeleteAllDbFromMap();
}

std::string SQLiteDB::ConcatDbFullPath(const std::string& dbFileName) const
{
#ifdef WIN32
    std::string dbFullPath = m_metaPath + Module::Win32PathUtil::Win32ToPosix(dbFileName);
    std::replace(dbFullPath.begin(), dbFullPath.end(), SLASH[0], BACKSLASH[0]);
    dbFullPath = Win32BackupEngineUtils::ExtenedPath(dbFullPath);
#else
    std::string dbFullPath = m_metaPath + dbFileName;
#endif
    DBGLOG("SQLiteDB concat full database path: %s", dbFullPath.c_str());
    return dbFullPath;
}

bool SQLiteDB::GetAliasForDbPath(string& dbFullPath) const
{
    std::string aliasForDbPath = m_metaAliasPath + Module::PATH_SEPARATOR + to_string(m_idGenerator->GenerateId());
    FSBackupUtils::RecurseCreateDirectory(aliasForDbPath);
    std::string newLinkFile = aliasForDbPath + Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    /* 不能使用symlink创建软链接，sqlite3_open_v2会替换软链接为原始路径，同样会导致超过sqlite的512长度路径限制，打开失败.
     * 硬链接S_ISLNK为false，sqlite3不会替换，使用link创建前，源文件必须存在.
     */
    if (!FSBackupUtils::CreateFile(dbFullPath)) {
        ERRLOG("Creating file %s failed.", dbFullPath.c_str());
        return false;
    }
    if (!FSBackupUtils::CreateLink(newLinkFile, dbFullPath)) {
        ERRLOG("Creating link failed. target: %s, link: %s", dbFullPath.c_str(), newLinkFile.c_str());
        return false;
    }
    INFOLOG("Creating link. target: %s, link: %s", dbFullPath.c_str(), newLinkFile.c_str());
    dbFullPath = newLinkFile;
    return true;
}

std::shared_ptr<SQLiteCoreInfo> SQLiteDB::Connect(const std::string& dbFile, uint32_t flags, bool isReconnect,
    std::shared_ptr<SQLiteCoreInfo> currSqlInfoPtr)
{
    sqlite3* pDB = nullptr;
    int32_t retryCnt = 0;
    std::string dbFullPath = ConcatDbFullPath(dbFile);
#ifndef WIN32
    if (dbFullPath.size() > MAX_PATH_LEN_SUPPORED_BY_SQLITE) {
        if (!GetAliasForDbPath(dbFullPath)) {
            return nullptr;
        }
    }
#endif
    int32_t ret = SQLITE_ERROR;
    while ((ret != SQLITE_OK) && (retryCnt++ < MAX_DB_OPEN_RETRY)) {
        DBGLOG("db open. dbFile: %s, retrying: %d", dbFullPath.c_str(), retryCnt);
#ifdef WIN32
        char vfsModuleName[] = "win32-longpath";
        ret = sqlite3_open_v2(dbFullPath.c_str(), &pDB, flags, vfsModuleName);
#else
        ret = sqlite3_open_v2(dbFullPath.c_str(), &pDB, flags, nullptr);
#endif
        if (ret != SQLITE_OK) {
            ERRLOG("db open failed. ret: %d, errMsg: %s, dbFile: %s, retrying: %d",
                ret, sqlite3_errmsg(pDB), dbFullPath.c_str(), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            continue;
        }
        sqlite3_busy_handler(pDB, SQLBusyCallback, static_cast<void*>(pDB));
        if ((flags & SQLITE_OPEN_CREATE) == SQLITE_OPEN_CREATE) {
            ret = CreateTable(pDB);
            if (ret != SQLITE_OK) {
                ERRLOG("Creat table failed. dbFile: %s, retrying: %d", dbFullPath.c_str(), retryCnt);
                sqlite3_close(pDB);
                Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
                continue;
            }
        }
    }
    if (ret != SQLITE_OK) {
        ERRLOG("db open failed or table creation failed.errno is: %d, dbFile: %s", ret, dbFullPath.c_str());
        return nullptr;
    }
    if (!isReconnect) {
        return std::make_shared<SQLiteCoreInfo>(pDB, dbFile);
    }
    currSqlInfoPtr->m_pDB = pDB;
    return currSqlInfoPtr;
}

int32_t SQLiteDB::CreateTable(sqlite3* pDB)
{
    char *zErrMsg = nullptr;
    int32_t ret = sqlite3_exec(pDB, "PRAGMA synchronous = OFF", nullptr, nullptr, &zErrMsg);
    if (ret != SQLITE_OK) {
        ERRLOG("Set PRAGMA synchronous = OFF failed. ret: %d, errMsg: %s", ret, zErrMsg);
        sqlite3_free(zErrMsg);
        return ret;
    }
    ret = sqlite3_exec(pDB, CREATETABLE.c_str(), nullptr, nullptr, &zErrMsg);
    if (ret != SQLITE_OK) {
        ERRLOG("Table creation open failed. ret: %d, errMsg: %s", ret, zErrMsg);
        sqlite3_free(zErrMsg);
        return ret;
    }
    return ret;
}

std::shared_ptr<SQLiteCoreInfo> SQLiteDB::PrepareDb(const std::string& dbFile, int flags)
{
    DBGLOG("Enter PrepareDb(). dbFile: %s, flags: %d", dbFile.c_str(), flags);

    std::lock_guard<std::mutex> lock(m_sqliteDbMtx);
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr {};
    sqlInfoPtr = QueryDbFromMap(dbFile);
    if (sqlInfoPtr == nullptr) {
        CheckAndDeleteAllDbFromMap();
        sqlInfoPtr = Connect(dbFile, flags);
        if (sqlInfoPtr == nullptr) {
            return nullptr;
        }
        AddDbToMap(dbFile, sqlInfoPtr);
    } else {
        DBGLOG("dbFile: %s already opened.", dbFile.c_str());
    }
    DBGLOG("Exit PrepareDb(). dbFile: %s", dbFile.c_str());
    return sqlInfoPtr;
}

int32_t SQLiteDB::DeleteAndPrepareDb(const std::string& dbFile,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr, int flags)
{
    DBGLOG("Enter DeleteAndPrepareDb(). dbFile: %s", dbFile.c_str());

    std::lock_guard<std::mutex> lock(m_sqliteDbMtx);
    sqlInfoPtr->Disconnect();
    if (Connect(dbFile, flags, true, sqlInfoPtr) == nullptr) {
        return FAILED;
    }
    return SUCCESS;
}

/*
 Step to use sqlite prepared statements:
 1. Call PrepareSqlStmt API - to create stmt object
 2. Call BindStepSql API - to bind values to stmt
 3. Call FinalizeSqlStmt API - to destroy stmt object
*/
int32_t SQLiteDB::PrepareSqlStmt(const std::string& dbFile, sqlite3_stmt** const sqlStmt,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const
{
    DBGLOG("Enter PrepareSqlStmt(). File: %s", dbFile.c_str());
    std::ostringstream buff;
#ifdef _OBS
    buff << "replace into " << SQLLITE_INDEX_TABLE << "(" << SQLITE_INDEX_TABLE_UUID << ","
         << SQLITE_INDEX_TABLE_FILENAME << "," << SQLITE_INDEX_TABLE_TYPE << ","
         << SQLITE_INDEX_TABLE_PARENT_PATH << "," << SQLITE_INDEX_TABLE_PARENT_UUID << ","
         << SQLITE_INDEX_TABLE_FILESIZE << "," << SQLITE_INDEX_TABLE_CREATE_TIME << ","
         << SQLITE_INDEX_TABLE_MODIFY_TIME << "," << SQLITE_INDEX_TABLE_EXTEND_INFO << ","
         << SQLITE_INDEX_TABLE_RES_TYPE << "," << SQLITE_INDEX_TABLE_RES_SUB_TYPE << ","
         << SQLITE_INDEX_TABLE_META_INFO << ") values(?,?,?,?,?,?,?,?,?,?,?,?);";
#else
    buff << "replace into " << SQLLITE_INDEX_TABLE << "(" << SQLITE_INDEX_TABLE_UUID << ","
         << SQLITE_INDEX_TABLE_FILENAME << "," << SQLITE_INDEX_TABLE_TYPE << ","
         << SQLITE_INDEX_TABLE_PARENT_PATH << "," << SQLITE_INDEX_TABLE_PARENT_UUID << ","
         << SQLITE_INDEX_TABLE_FILESIZE << "," << SQLITE_INDEX_TABLE_CREATE_TIME << ","
         << SQLITE_INDEX_TABLE_MODIFY_TIME << "," << SQLITE_INDEX_TABLE_EXTEND_INFO << ","
         << SQLITE_INDEX_TABLE_RES_TYPE << "," << SQLITE_INDEX_TABLE_RES_SUB_TYPE
         << ") values(?,?,?,?,?,?,?,?,?,?,?);";
#endif
    std::string strSql = buff.str();

    int32_t ret;
    if (sqlInfoPtr == nullptr) {
        ERRLOG("BindStepSql failed, sqlInfoPtr is null");
        return FAILED;
    }
    ret = sqlInfoPtr->BeginTrans();
    if (ret != SUCCESS) {
        ERRLOG("BeginTrans failed.");
        return FAILED;
    }
    *sqlStmt = sqlInfoPtr->SqlPrepare(strSql);
    if (*sqlStmt == nullptr) {
        ERRLOG("SqlPrepare failed.");
        return FAILED;
    }
    DBGLOG("Exit PrepareSqlStmt Success");
    return SUCCESS;
}

int32_t SQLiteDB::BindStepSql(sqlite3_stmt* const sqlStmt, DbParamStream& dps,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const
{
    if (sqlInfoPtr == nullptr) {
        ERRLOG("BindStepSql failed, sqlInfoPtr is null");
        return FAILED;
    }
    int32_t ret = sqlInfoPtr->SqlBind(sqlStmt, dps);
    if (ret != SUCCESS) {
        ERRLOG("Bind params failed. ret=%d", ret);
        return FAILED;
    }
    ret = sqlInfoPtr->SqlStepExecute(sqlStmt);
    if (ret != SUCCESS) {
        ERRLOG("Sql execute failed. ret=%d", ret);
        return FAILED;
    }
    dps.Clear();
    return SUCCESS;
}

int32_t SQLiteDB::FinalizeSqlStmt(sqlite3_stmt* const sqlStmt,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const
{
    if (sqlInfoPtr == nullptr) {
        ERRLOG("FinalizeSqlStmt failed, sqlInfoPtr is null");
        return FAILED;
    }
    if (sqlInfoPtr->FinalizeSqlStmt(sqlStmt) != SUCCESS) {
        ERRLOG("FinalizeSqlStmt failed");
        /* Callers/Users - Be aware */
        if (sqlInfoPtr->CommitTrans() != SUCCESS) {
            ERRLOG("FinalizeSqlStmt failed, subsequent commit failed");
        }
        return FAILED;
    }
    int32_t ret = sqlInfoPtr->CommitTrans();
    if (ret != SUCCESS) {
        ERRLOG("CommitTrans failed. ret=%d", ret);
        return FAILED;
    }
    return SUCCESS;
}

int32_t SQLiteDB::QueryTable(QueryTableArgs &queryTableArgs, DbParamStream& dps,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr) const
{
    DBGLOG("Enter QueryTable dbFile: %s", queryTableArgs.dbFile.c_str());
    int32_t ret;

    /* Allow only one thread to operate on a particular sqlite db */
    std::lock_guard<std::mutex> lock(sqlInfoPtr->m_sqliteCoreInfoMutex);
    sqlite3_stmt* stmt = sqlInfoPtr->SqlPrepare(queryTableArgs.sql);
    if (stmt == nullptr) {
        ERRLOG("Prepare for query table failed.");
        return FAILED;
    }

    ret = sqlInfoPtr->SqlBind(stmt, dps);
    if (ret != SUCCESS) {
        ERRLOG("Bind params failed.");
        sqlInfoPtr->FinalizeSqlStmt(stmt);
        return FAILED;
    }

    ret = sqlInfoPtr->SqlQuery(stmt, queryTableArgs.readBuff, queryTableArgs.rowCount,
        queryTableArgs.colCount);
    if (ret != SUCCESS) {
        ERRLOG("Sql query failed.");
        sqlInfoPtr->FinalizeSqlStmt(stmt);
        return FAILED;
    }

    if (sqlInfoPtr->FinalizeSqlStmt(stmt) != SUCCESS) {
        ERRLOG("FinalizeSqlStmt failed");
        return FAILED;
    }
    DBGLOG("Exit QueryTable.");
    return SUCCESS;
}

int32_t SQLiteDB::Disconnect(const std::string& dbFile, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr)
{
    DBGLOG("Disconnect, dbFile: %s", dbFile.c_str());
    if (sqlInfoPtr == nullptr) {
        return SUCCESS;
    }
    DeleteFromMap(dbFile);
    return sqlInfoPtr->Disconnect();
}

void SQLiteDB::AddDbToMap(const std::string& dbFile, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr)
{
    DBGLOG("DB entry added to map. dbFile: %s", dbFile.c_str());
    std::lock_guard<std::mutex> lock(m_dbMapMtx);
    m_dBMap.emplace(make_pair(dbFile, sqlInfoPtr));
}

std::shared_ptr<SQLiteCoreInfo> SQLiteDB::QueryDbFromMap(const std::string& dbFile)
{
    std::lock_guard<std::mutex> lock(m_dbMapMtx);
    auto it = m_dBMap.find(dbFile);
    if (it != m_dBMap.end()) {
        return it->second;
    }

    return nullptr;
}

void SQLiteDB::DeleteFromMap(const std::string& dbFile)
{
    DBGLOG("DB entry delete from map. dbFile: %s", dbFile.c_str());
    std::lock_guard<std::mutex> lock(m_dbMapMtx);
    auto it = m_dBMap.find(dbFile);
    if (it != m_dBMap.end()) {
        m_dBMap.erase(dbFile);
    }
    return;
}

void SQLiteDB::DeleteAllDbFromMap()
{
    std::lock_guard<std::mutex> lock(m_dbMapMtx);
    auto it = m_dBMap.begin();
    while (it != m_dBMap.end()) {
        it = m_dBMap.erase(it);
    }
}

void SQLiteDB::CheckAndDeleteAllDbFromMap()
{
    /* Delete all old db handles to reduce total open file handles */
    std::lock_guard<std::mutex> lock(m_dbMapMtx);
    if (m_dBMap.size() < MAX_OPEN_DB_FILE_HANDLES) {
        return;
    }
    auto it = m_dBMap.begin();
    while (it != m_dBMap.end()) {
        it = m_dBMap.erase(it);
    }
}

string SQLiteDB::GetMetaPath()
{
    return m_metaPath;
}

void IndexDetails::Uint64ToString(uint64_t integerVal, std::string& strVal) const
{
    std::stringstream ss;
    ss << integerVal;
    ss >> strVal;
}

void IndexDetails::StringToUint64(std::string strVal, uint64_t& integerVal) const
{
    std::stringstream ss;
    ss << strVal;
    ss >> integerVal;
}

int32_t IndexDetails::InsertIndexInfo(std::shared_ptr<SQLiteDB> sqliteDB, sqlite3_stmt* const sqlStmt,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr)
{
    /* PrepareSqlStmt API is mandatory to call before this */

    DbParamStream dps;
    DbParam dp;

    dp = m_uuid;
    dps << dp;
    dp = m_actualFileName;
    dps << dp;
    dp = m_type;
    dps << dp;
    dp = m_aggregateFileName;
    dps << dp;
    dp = m_parentUUID;
    dps << dp;
    dp = m_actualFileSize;
    dps << dp;
    dp = m_createTime;
    dps << dp;
    dp = m_modifyTime;
    dps << dp;
    std::ostringstream temp;
    temp << m_metaFileName << STR_COMMA << m_offset << STR_COMMA << m_aggregatedFileSize;
    dp = temp.str();
    dps << dp;
    dp = m_resType;
    dps << dp;
    dp = m_resSubType;
    dps << dp;
#ifdef _OBS
    dp = m_metaData;
    dps << dp;
#endif
    int32_t ret = sqliteDB->BindStepSql(sqlStmt, dps, sqlInfoPtr);
    if (ret != SUCCESS) {
        ERRLOG("db.BindStepSql failed, ret = %d.", ret);
    }

    return ret;
}

int32_t IndexDetails::DeleteIndexInfoByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
    const std::string& normalFileName)
{
    int createFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
    std::shared_ptr<SQLiteCoreInfo> sqlInfo = sqliteDB->PrepareDb(dbFile, createFlags);
    if (sqlInfo == nullptr) {
        return FAILED;
    }

    int32_t ret = sqlInfo->BeginTrans();
    if (ret != SUCCESS) {
        ERRLOG("Begin trans failed.");
        return ret;
    }

    std::string sql = "DELETE FROM " + SQLLITE_INDEX_TABLE + " WHERE " + SQLITE_INDEX_TABLE_FILENAME + " == ?;";
    sqlite3_stmt* sqlStmt = sqlInfo->SqlPrepare(sql);
    if (sqlStmt == nullptr) {
        ERRLOG("Sql prepare failed.");
        return FAILED;
    }

    DbParamStream dps;
    DbParam dp = normalFileName;
    dps << dp;
    ret = sqliteDB->BindStepSql(sqlStmt, dps, sqlInfo);
    if (ret != SUCCESS) {
        ERRLOG("Bind step sql failed, ret = %d.", ret);
        return ret;
    }

    ret = sqliteDB->FinalizeSqlStmt(sqlStmt, sqlInfo);
    if (ret != SUCCESS) {
        ERRLOG("Finalize sql stmt failed, ret = %d.", ret);
        return ret;
    }

    return SUCCESS;
}

void IndexDetails::FillIndexDetailsParams(QueryTableArgs &queryTableArgs, IndexDetails &indexInfo)
{
    std::string strTmp;
    queryTableArgs.readBuff >> strTmp;
    indexInfo.m_uuid = strTmp;
    queryTableArgs.readBuff >> strTmp;
    indexInfo.m_actualFileName = strTmp;
    queryTableArgs.readBuff >> strTmp;
    indexInfo.m_type = strTmp;
    queryTableArgs.readBuff >> strTmp;
    indexInfo.m_aggregateFileName = strTmp;

    // ignore parent uuid
    queryTableArgs.readBuff >> strTmp;

    // read size
    queryTableArgs.readBuff >> strTmp;
    StringToUint64(strTmp, indexInfo.m_actualFileSize);

    // ignore create time & modify time
    queryTableArgs.readBuff >> strTmp;
    StringToUint64(strTmp, indexInfo.m_createTime);
    queryTableArgs.readBuff >> strTmp;
    StringToUint64(strTmp, indexInfo.m_modifyTime);

    // read extended info as ("metafilename , metaoffset, aggregatedFileSize" format)
    queryTableArgs.readBuff >> strTmp;

    std::stringstream ss(strTmp);
    std::string s;
    std::vector<std::string> out;
    while (std::getline(ss, s, CHAR_COMMA)) {
        out.push_back(s);
    }

    indexInfo.m_metaFileName = out[INDEX_META_FILE_NAME];
    StringToUint64(out[INDEX_META_FILE_OFFSET], indexInfo.m_offset);
    StringToUint64(out[INDEX_AGGR_FILE_SIZE], indexInfo.m_aggregatedFileSize);

    // ignore res_type & res_sub_type
    queryTableArgs.readBuff >> strTmp;
    indexInfo.m_resType = strTmp;
    queryTableArgs.readBuff >> strTmp;
#ifdef _OBS
    queryTableArgs.readBuff >> indexInfo.m_metaData;
#endif
}

int32_t IndexDetails::QueryAllIndexInfo(std::shared_ptr<SQLiteDB> sqliteDB,
    const std::string& dbFile, std::vector<IndexDetails>& vecIndexInfo,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore)
{
    DBGLOG("Enter QueryAllIndexInfo");
    ostringstream buff;
    buff << "select * from " << SQLLITE_INDEX_TABLE;
    std::string sql = buff.str();

    DbParamStream dps;
    QueryTableArgs queryTableArgs;
    queryTableArgs.sql = sql;
    queryTableArgs.dbFile = dbFile;

    int32_t retVal = sqliteDB->QueryTable(queryTableArgs, dps, sqlInfoPtrRestore);
    if (retVal != SUCCESS) {
        ERRLOG("db.QueryTable failed, ret = %d.", retVal);
        return retVal;
    }

    for (int32_t iRowVal = 0; iRowVal < queryTableArgs.rowCount; ++iRowVal) {
        IndexDetails indexInfo;
        FillIndexDetailsParams(queryTableArgs, indexInfo);
        vecIndexInfo.push_back(indexInfo);
    }
    DBGLOG("Exit QueryAllIndexInfo");
    return SUCCESS;
}

int64_t IndexDetails::QueryRecordNum(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile, int64_t& count)
{
    DBGLOG("Enter QueryRecordNum");

    std::string sql = "select count (*) from " + SQLLITE_INDEX_TABLE;
    int readFlags = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = sqliteDB->PrepareDb(dbFile, readFlags);
    if (sqlInfoPtr == nullptr) {
        return FAILED;
    }

    std::lock_guard<std::mutex> lock(sqlInfoPtr->m_sqliteCoreInfoMutex);
    sqlite3_stmt* stmt = sqlInfoPtr->SqlPrepare(sql);
    if (stmt == nullptr) {
        ERRLOG("Prepare for query table failed.");
        sqliteDB->Disconnect(dbFile, sqlInfoPtr);
        return FAILED;
    }

    int32_t ret = SQLITE_OK;
    do {
        ret = sqlite3_step(stmt);
    } while (ret != SQLITE_ROW);

    count = sqlite3_column_int64(stmt, 0);

    if (sqlInfoPtr->FinalizeSqlStmt(stmt) != SUCCESS) {
        ERRLOG("FinalizeSqlStmt failed");
        sqliteDB->Disconnect(dbFile, sqlInfoPtr);
        return FAILED;
    }

    sqliteDB->Disconnect(dbFile, sqlInfoPtr);
    DBGLOG("Query records suceess, record num: %ld", count);
    return SUCCESS;
}

int32_t IndexDetails::QueryIndexInfoByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
    std::vector<IndexDetails>& vecIndexInfo, const std::string& normalFileName, const std::string type)
{
    DBGLOG("Enter QueryIndexInfoByName");
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = sqliteDB->PrepareDb(dbFile, READ_DB_FLAGS);
    if (sqlInfoPtr == nullptr) {
        ERRLOG("PrepareDb failed.");
        return FAILED;
    }

    ostringstream buff;
    buff << "select * from " << SQLLITE_INDEX_TABLE << " where " << SQLITE_INDEX_TABLE_FILENAME
            << " == ?" << " and " << SQLITE_INDEX_TABLE_TYPE << " == ?;";;
    std::string sql = buff.str();

    QueryTableArgs queryTableArgs;
    queryTableArgs.dbFile = dbFile;
    queryTableArgs.sql = sql;

    DbParamStream dps;
    DbParam dp = normalFileName;
    dps << dp;
    dp = type;
    dps << dp;

    int32_t retCode = sqliteDB->QueryTable(queryTableArgs, dps, sqlInfoPtr);
    if (retCode != SUCCESS) {
        ERRLOG("db.QueryTable failed, ret = %d.", retCode);
        return retCode;
    }

    for (int32_t iRowCount = 0; iRowCount < queryTableArgs.rowCount; ++iRowCount) {
        IndexDetails indexInfo;
        FillIndexDetailsParams(queryTableArgs, indexInfo);
        vecIndexInfo.push_back(indexInfo);
    }

    return SUCCESS;
}

int32_t IndexDetails::QueryZipNameNSizeByName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
    const std::string& normalFileName, AggSqlRestoreQueryInfo &info, std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore)
{
    DBGLOG("Query blob file, dbFile: %s normalFileName: %s", dbFile.c_str(), normalFileName.c_str());
    std::string strTmp;
    ostringstream buff;
    buff << "select " << SQLITE_INDEX_TABLE_PARENT_PATH <<" , " << SQLITE_INDEX_TABLE_FILESIZE <<" , "
        << SQLITE_INDEX_TABLE_EXTEND_INFO << " from "
        << SQLLITE_INDEX_TABLE  << " where " << SQLITE_INDEX_TABLE_FILENAME  << " == ?" << " and "
        << SQLITE_INDEX_TABLE_TYPE << " == ?;";;
    std::string sql = buff.str();

    std::string type = "f";
    DbParamStream dps;
    DbParam dp = normalFileName;
    dps << dp;
    dp = type;
    dps << dp;

    QueryTableArgs queryTableArgs;
    queryTableArgs.dbFile = dbFile;
    queryTableArgs.sql = sql;

    int32_t ret = sqliteDB->QueryTable(queryTableArgs, dps, sqlInfoPtrRestore);
    if (ret != SUCCESS) {
        ERRLOG("db.QueryTable failed, ret = %d.", ret);
        return ret;
    }

    for (int32_t iRow = 0; iRow < queryTableArgs.rowCount; ++iRow) {
        queryTableArgs.readBuff >> info.blobFileName;

        queryTableArgs.readBuff >> strTmp;
        StringToUint64(strTmp, info.normalFileSize);

        // read extended info as ("metafilename , metaoffset, aggregatedFileSize" format)
        queryTableArgs.readBuff >> strTmp;
        std::stringstream ss(strTmp);
        std::string s;
        std::vector<std::string> out;
        while (std::getline(ss, s, CHAR_COMMA)) {
            out.push_back(s);
        }
        StringToUint64(out[INDEX_AGGR_FILE_SIZE], info.blobFileSize);
        StringToUint64(out[INDEX_META_FILE_OFFSET], info.fileOffset);
        DBGLOG("iRow %d, blobFileName = %s, normalFileSize = %llu, blobFileSize = %lld, fileOffset = %lld.",
               iRow, info.blobFileName.c_str(), info.normalFileSize, info.blobFileSize, info.fileOffset);
    }

    return SUCCESS;
}

int32_t IndexDetails::QueryNormalFilesByAggName(std::shared_ptr<SQLiteDB> sqliteDB, const std::string& dbFile,
    std::vector<AggSqlRestoreInfo>& vecNormalFilesList, const std::string& aggFileName,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore)
{
    DBGLOG("Enter query, aggregateFileName:%s", aggFileName.c_str());
    ostringstream buff;
    buff << "select "<< SQLITE_INDEX_TABLE_FILENAME <<" , " << SQLITE_INDEX_TABLE_FILESIZE <<" , "
        << SQLITE_INDEX_TABLE_EXTEND_INFO<<" from "
        << SQLLITE_INDEX_TABLE << " where "  << SQLITE_INDEX_TABLE_PARENT_PATH << " == ?;";
    std::string sql = buff.str();

    DbParamStream dps;
    DbParam dp = aggFileName;
    dps << dp;
    QueryTableArgs queryTableArgs;
    queryTableArgs.dbFile = dbFile;
    queryTableArgs.sql = sql;

    int32_t retValue = sqliteDB->QueryTable(queryTableArgs, dps, sqlInfoPtrRestore);
    if (retValue != SUCCESS) {
        ERRLOG("db.QueryTable failed, ret = %d.", retValue);
        return retValue;
    }

    for (int32_t iRow = 0; iRow < queryTableArgs.rowCount; ++iRow) {
        AggSqlRestoreInfo resInfo;
        std::string strTmp;
        queryTableArgs.readBuff >> resInfo.normalFileName;

        queryTableArgs.readBuff >> strTmp;
        StringToUint64(strTmp, resInfo.normalFileSize);

        // read extended info as ("metafilename , metaoffset, aggregatedFileSize" format)
        queryTableArgs.readBuff >> strTmp;
        std::stringstream ss(strTmp);
        std::string s;
        std::vector<std::string> out;
        while (std::getline(ss, s, CHAR_COMMA)) {
            out.push_back(s);
        }
        StringToUint64(out[INDEX_META_FILE_OFFSET], resInfo.normalFileOffset);
        DBGLOG("iRow %d, normalFileName = %s, normalFileSize = %llu, normalFileOffset = %lld.",
               iRow, resInfo.normalFileName.c_str(), resInfo.normalFileSize, resInfo.normalFileOffset);

        vecNormalFilesList.push_back(resInfo);
    }
    DBGLOG("Exit query.");
    return SUCCESS;
}

/*
 * step1: 由文件找到对应的聚合文件名；
 * step2: 找到该聚合文件中包含的所有聚合前的文件名
 */
int32_t IndexDetails::QueryNormalFilesByName(std::shared_ptr<SQLiteDB> sqliteDB, std::string& fileName,
    std::string& dbFile, AggSqlRestoreQueryInfo &info, std::vector<AggSqlRestoreInfo> &vecNormalFiles)
{
    string dbFullPath = sqliteDB->GetMetaPath() + dbFile;

    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtrRestore = sqliteDB->PrepareDb(dbFile, READ_DB_FLAGS);
    if (sqlInfoPtrRestore == nullptr) {
        ERRLOG("PrepareDb failed for dbFile: %s fileName: %s", dbFullPath.c_str(), fileName.c_str());
        return -1;
    }

    // get the blob file name & size using the normal file name
    if (QueryZipNameNSizeByName(sqliteDB, dbFile, fileName, info, sqlInfoPtrRestore) != 0) {
        ERRLOG("QueryZipNameNSizeByName for dbFile: %s fileName: %s ", dbFullPath.c_str(), fileName.c_str());
        sqliteDB->Disconnect(dbFile, sqlInfoPtrRestore);
        return -1;
    }

    if (info.blobFileName.empty()) {
        return 0;
    }

    // 获取聚合文件blob中包含的所有小文件，这些小文件只会在一个sqlite中，因为sqlite是按备份子任务生成的
    if (QueryNormalFilesByAggName(sqliteDB, dbFile, vecNormalFiles, info.blobFileName, sqlInfoPtrRestore) != 0) {
        ERRLOG("Query normal files failed, path: %s blobFileName: %s", dbFullPath.c_str(), info.blobFileName.c_str());
        sqliteDB->Disconnect(dbFile, sqlInfoPtrRestore);
        return -1;
    }

    sqliteDB->Disconnect(dbFile, sqlInfoPtrRestore);
    return 0;
}

int32_t IndexDetails::MergeDbFile(std::shared_ptr<SQLiteDB> sqliteDB,
    const std::string& dstDbFile, const std::string& srcDbFile)
{
    DBGLOG("Merge db file %s", srcDbFile.c_str());
    int createFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
    std::shared_ptr<SQLiteCoreInfo> dstSqlInfo = sqliteDB->PrepareDb(dstDbFile, createFlags);
    if (dstSqlInfo == nullptr) {
        return FAILED;
    }

    std::string srcFullPath = sqliteDB->ConcatDbFullPath(srcDbFile);
#ifndef WIN32
    if (srcFullPath.size() > MAX_PATH_LEN_SUPPORED_BY_SQLITE) {
        if (!sqliteDB->GetAliasForDbPath(srcFullPath)) {
            return FAILED;
        }
    }
#endif

    std::string aliasName = "dba";
    std::string aliasTable = aliasName + "." + SQLLITE_INDEX_TABLE;
    ostringstream buff;
    buff << "ATTACH '" << srcFullPath << "' as " << aliasName << ";" <<
            "BEGIN;" <<
            "INSERT OR IGNORE INTO " << SQLLITE_INDEX_TABLE << " SELECT * FROM " << aliasTable << ";" <<
            "COMMIT;" <<
            "detach database " << aliasName << ";";
    std::string sql = buff.str();
    std::lock_guard<std::mutex> lock(dstSqlInfo->m_sqliteCoreInfoMutex);
    int32_t ret = sqlite3_exec(dstSqlInfo->m_pDB, sql.c_str(), nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        ERRLOG("ATTACH failed for %s, ret=%d, %s", srcFullPath.c_str(), ret, sqlite3_errmsg(dstSqlInfo->m_pDB));
        std::string dettach = "detach database " + aliasName + ";";
        sqlite3_exec(dstSqlInfo->m_pDB, dettach.c_str(), nullptr, nullptr, nullptr);
        sqliteDB->Disconnect(dstDbFile, dstSqlInfo);
        return ret;
    }

    return SUCCESS;
}
