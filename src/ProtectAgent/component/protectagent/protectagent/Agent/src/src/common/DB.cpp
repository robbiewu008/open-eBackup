/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file db.cpp
 * @brief  The implemention about db operations
 * @version 1.0.0.0
 * @date 2015-02-06
 * @author WuZhanglin 00281912
 */
#include "common/DB.h"
#include "common/Log.h"
#include "common/Path.h"

using namespace std;

CDB CDB::m_Instance;
DWSDB DWSDB::m_Instance;

namespace {
const mp_string AGENT_DB_FILE = "AgentDB.db";
const mp_string DWS_DB_FILE = "DwsDB.db";
}  // namespace

namespace {
int BusyCallback(void* ptr, int retryNum)
{
    static const mp_int32 retryMaxTime = 30;
    if (retryNum == retryMaxTime) {
        COMMLOG(OS_LOG_WARN, "SQLITE_BUSY retryNum=%d", retryNum);
        return 0;
    }
    static const mp_int32 retrySleepTime = 100;
    sqlite3_sleep(retrySleepTime);
    return 1;
}
}  // namespace

/* ------------------------------------------------------------
Function Name:DBReader
Description  :DBReader构造函数
Others       :------------------------------------------------------------- */
DBReader::DBReader()
{
    m_lstResult.clear();
}

/* ------------------------------------------------------------
Function Name:~DBReader
Description  :DBReader析构函数
Others       :------------------------------------------------------------- */
DBReader::~DBReader()
{
    m_lstResult.clear();
}

/* ------------------------------------------------------------
Function Name:Clear
Description  :清除list成员变量内容
Others       :------------------------------------------------------------- */
mp_void DBReader::Clear()
{
    m_lstResult.clear();
}

/* ------------------------------------------------------------
Function Name:Empty
Description  :判断list成员变量是否为空
Others       :------------------------------------------------------------- */
mp_bool DBReader::Empty()
{
    return m_lstResult.empty();
}

/* ------------------------------------------------------------
Function Name:operator<<
Description  :重载<<操作符
Others       :------------------------------------------------------------- */
mp_string DBReader::operator<<(mp_string& strResult)
{
    m_lstResult.push_back(strResult);
    return strResult;
}

/* ------------------------------------------------------------
Function Name:operator>>
Description  :重载>>操作符
Others       :------------------------------------------------------------- */
mp_string DBReader::operator>>(mp_string& strResult)
{
    // codedex误报CHECK_CONTAINER_EMPTY,容器ite在之前的代码能保证不为空，此处可以不判断
    list<mp_string>::iterator ite = m_lstResult.begin();
    if (m_lstResult.end() != ite) {
        strResult = *ite;
        m_lstResult.erase(ite);
    } else {
        COMMLOG(OS_LOG_WARN, "%s", "There has no param.");
    }
    return strResult;
}

/* ------------------------------------------------------------
Function Name:Connect
Description  :连接sqlite数据库
Others       :------------------------------------------------------------- */
mp_int32 DB::Connect(const mp_string& dbFile, mp_bool isTrans)
{
    mp_int32 iRet;

    if (!isTrans && m_pTransDB) {  // 使用事务中打开的数据库连接
        m_pDB = m_pTransDB;
        return MP_SUCCESS;
    }
    // 连接之前首先释放以前的连接
    iRet = Disconnect(isTrans);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "close db connection failed. errno is %d:", iRet);
        return MP_FAILED;
    }

    // 打开sqlite连接
    mp_string strDbFile = dbFile;
    iRet = (isTrans ? sqlite3_open(strDbFile.c_str(), &m_pTransDB) : sqlite3_open(strDbFile.c_str(), &m_pDB));
    if (isTrans && iRet != SQLITE_OK) {
        // 记录日志
        COMMLOG(OS_LOG_ERROR, "db open failed.errno is: %d, %s.", iRet, sqlite3_errmsg(m_pTransDB));
        return MP_FAILED;
    } else if (!isTrans && iRet != SQLITE_OK) {
        // 记录日志
        COMMLOG(OS_LOG_ERROR, "db open failed.errno is: %d, %s.", iRet, sqlite3_errmsg(m_pDB));
        return MP_FAILED;
    }
    if (isTrans) {
        sqlite3_busy_handler(m_pTransDB, BusyCallback, (void*)m_pTransDB);
    } else {
        sqlite3_busy_handler(m_pDB, BusyCallback, (void*)m_pDB);
    }

    return MP_SUCCESS;
}

// 返回成功表示不需要断开与数据库的连接
mp_int32 DB::CheckNeedDisconnect(mp_bool isTrans)
{
    if (!isTrans && m_pTransDB != NULL) {  // TRANS事务的连接还没有断开
        return MP_SUCCESS;
    }

    // 非事务/事务两种模式下都已经释放了连接
    if ((!isTrans && m_pDB == NULL) || (isTrans && m_pTransDB == NULL)) {
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

/* ------------------------------------------------------------
Function Name:Disconnect
Description  :断开连接sqlite数据库
Others       :------------------------------------------------------------- */
mp_int32 DB::Disconnect(mp_bool isTrans)
{
    if (CheckNeedDisconnect(isTrans) == MP_SUCCESS) {
        return MP_SUCCESS;
    }
    mp_int32 iRet = (isTrans ? sqlite3_close(m_pTransDB) : sqlite3_close(m_pDB));
    if (isTrans && iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "close db connection failed.errno is:%d, %s", iRet, sqlite3_errmsg(m_pTransDB));
        return MP_FAILED;
    } else if (!isTrans && iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "close db connection failed.errno is:%d, %s", iRet, sqlite3_errmsg(m_pDB));
        return MP_FAILED;
    }

    m_pTransDB = NULL;
    m_pDB = NULL;  // m_pTransDB != NULL时, 会有m_pDB = m_pTransDB
    return MP_SUCCESS;
}

// 采用预编译方式进行sqlite访问，
// 首先需要使用sqlite3_prepare_v2对sql语句进行预编译
// 预编译后会返回一个sqlite3_stmt指针
// 然后将要操作的内容插入到预编译后的sql语句中打问号的字段(sqlite_bind_*函数)
// 最后使用sqlite_step执行
sqlite3_stmt* DB::SqlPrepare(const mp_string& sql)
{
    // CodeDex误报，SQL Injection
    // SQL语句是内部写死的，非用户输入
    sqlite3_stmt* stmt = NULL;
    mp_int32 iRet = sqlite3_prepare_v2(m_pDB, sql.c_str(), sql.size(), &stmt, NULL);
    if (iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "sqlite3_prepare_v2 DB failed, iRet=%d, %s.",
                iRet, sqlite3_errmsg(m_pDB));
        if (MP_SUCCESS != Disconnect()) {
            COMMLOG(OS_LOG_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        }
        return NULL;
    }
    return stmt;
}

mp_int32 DB::SqlBind(sqlite3_stmt& stmt, DbParamStream& dps)
{
    if (!m_stringList.empty()) {
        m_stringList.clear();
    }
    mp_int32 iRet;
    for (mp_int32 i = 1; !dps.Empty(); i++) {
        DbParam dp;
        dps >> dp;

        switch (dp.m_type) {
            case DB_PARAM_TYPE_INT32:
                iRet = sqlite3_bind_int(&stmt, i, atoint32(dp.m_value.c_str()));
                break;
            case DB_PARAM_TYPE_UINT32:
            case DB_PARAM_TYPE_INT64:
            case DB_PARAM_TYPE_UINT64:
                iRet = sqlite3_bind_int64(&stmt, i, atoint64(dp.m_value.c_str()));
                break;
            case DB_PARAM_TYPE_STRING:
                m_stringList.push_front(dp.m_value);
                iRet = sqlite3_bind_text(&stmt, i, m_stringList.front().c_str(), m_stringList.front().size(), NULL);
                break;
            default:
                iRet = SQLITE_ERROR;
        }

        if (iRet != SQLITE_OK) {
            COMMLOG(OS_LOG_ERROR, "sqlite3_bind_* DB failed, iRet=%d, %s.",
                    iRet, sqlite3_errmsg(m_pDB));
            FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_bind_* DB failed");
        }
    }

    return MP_SUCCESS;
}

mp_int32 DB::SqlExecute(sqlite3_stmt& stmt)
{
    mp_int32 iRet = sqlite3_step(&stmt);
    if (iRet != SQLITE_DONE) {
        COMMLOG(OS_LOG_ERROR, "sqlite3_step failed, iRet=%d, %s.",
                iRet, sqlite3_errmsg(m_pDB));
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_step failed");
    }

    if (!m_stringList.empty()) {
        m_stringList.clear();
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:ExecSql
Description  :预编译方式执行sql语句
Others       :------------------------------------------------------------- */
mp_int32 DB::ExecSql(const mp_string& dbFile, const mp_string& strSql, DbParamStream& dps)
{
    COMMLOG(OS_LOG_DEBUG, "Begin Exec sql.");
    mp_int32 iRet = Connect(dbFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Connect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    sqlite3_stmt* sqlStmt = SqlPrepare(strSql);
    if (sqlStmt == NULL) {
        return MP_FAILED;
    }

    iRet = SqlBind(*sqlStmt, dps);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Bind params failed.");
        return MP_FAILED;
    }

    iRet = SqlExecute(*sqlStmt);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Sql execute failed.");
        return MP_FAILED;
    }

    if (sqlite3_finalize(sqlStmt) != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "sqlite3_finalize failed");
    }

    iRet = Disconnect();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    dps.Clear();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:SqlQuery
Description  :预编译方式QueryTable子函数
Others       :------------------------------------------------------------- */
mp_int32 DB::SqlQuery(sqlite3_stmt& stmt, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    mp_int32 colCount = sqlite3_column_count(&stmt);  // 列数
    if (colCount <= 0) {
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_column_count return <= 0");
    }
    mp_int32 rowCount = 0;  // 行数
    mp_int32 iRet = 0;
    while ((iRet = sqlite3_step(&stmt)) == SQLITE_ROW) {
        for (mp_int32 i = 0; i < colCount; i++) {
            mp_string text;
            const char* temp = (const char*)sqlite3_column_text(&stmt, i);
            if (temp != NULL) {
                text = temp;
            } else {
                COMMLOG(OS_LOG_ERROR, "sqlite3_column_text null, col=%d.", i);
            }
            readBuff << text;
        }
        rowCount++;
    }

    if (iRet != SQLITE_DONE) {
        COMMLOG(OS_LOG_ERROR, "sqlite3_step not return SQLITE_DONE, iRet=%d, %s.",
                iRet, sqlite3_errmsg(m_pDB));
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_step not return SQLITE_DONE");
    }

    iRowCount = rowCount;
    iColCount = colCount;
    if (!m_stringList.empty()) {
        m_stringList.clear();
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:QueryTable
Description  :预编译方式执行sqlite获取某条表数据
Others       :------------------------------------------------------------- */
mp_int32 DB::QueryTable(const mp_string& dbFile,
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    COMMLOG(OS_LOG_DEBUG, "Begin query table.");
    mp_int32 iRet = Connect(dbFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Connect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    sqlite3_stmt* stmt = SqlPrepare(strSql);
    if (stmt == NULL) {
        COMMLOG(OS_LOG_ERROR, "Prepare for query table failed.");
        return MP_FAILED;
    }

    iRet = SqlBind(*stmt, dps);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Bind params failed.");
        return MP_FAILED;
    }

    iRet = SqlQuery(*stmt, readBuff, iRowCount, iColCount);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Sql query failed.");
        return MP_FAILED;
    }

    if (sqlite3_finalize(stmt) != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "sqlite3_finalize failed");
    }

    iRet = Disconnect();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 开启事务
Input        : 无
Output       : 无
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 DB::BeginTrans(const mp_string& dbFile)
{
    COMMLOG(OS_LOG_DEBUG, "Begin exec begin transaction.");

    mp_int32 iRet = Connect(dbFile, MP_TRUE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Connect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    if (m_pTransDB == NULL) {
        COMMLOG(OS_LOG_ERROR, "Trans sqlite connection is null.");
        return MP_FAILED;
    }

    iRet = sqlite3_exec(m_pTransDB, "BEGIN", NULL, NULL, NULL);
    if (iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "Begin transaction failed, iRet=%d, %s.", iRet, sqlite3_errmsg(m_pDB));
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin transaction succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 回滚事务
Input        : 无
Output       : 无
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 DB::RollbackTrans()
{
    COMMLOG(OS_LOG_DEBUG, "Begin rollback transaction.");

    if (m_pTransDB == NULL) {
        COMMLOG(OS_LOG_ERROR, "Trans sqlite connection is null.");
        return MP_FAILED;
    }

    mp_int32 iRet = sqlite3_exec(m_pTransDB, "ROLLBACK", NULL, NULL, NULL);
    if (iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "Rollback transaction failed, iRet=%d, %s.", iRet, sqlite3_errmsg(m_pDB));
        (mp_void)Disconnect(MP_TRUE);
        return iRet;
    }

    iRet = Disconnect(MP_TRUE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Rollback transaction succ.");
    return iRet;
}

/*------------------------------------------------------------
Description  : 提交事务
Input        : 无
Output       : 无
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 DB::CommitTrans()
{
    COMMLOG(OS_LOG_DEBUG, "Begin commit transaction.");

    if (m_pTransDB == NULL) {
        COMMLOG(OS_LOG_ERROR, "Trans sqlite connection is null.");
        return MP_FAILED;
    }

    mp_int32 iRet = sqlite3_exec(m_pTransDB, "COMMIT", NULL, NULL, NULL);
    if (iRet != SQLITE_OK) {
        COMMLOG(OS_LOG_ERROR, "Commit transaction failed, iRet=%d, %s.", iRet, sqlite3_errmsg(m_pDB));
        (mp_void)Disconnect(MP_TRUE);
        return iRet;
    }

    iRet = Disconnect(MP_TRUE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Commit transaction succ.");
    return iRet;
}

mp_int32 CDB::ExecSql(const mp_string& strSql, DbParamStream& dpl)
{
    CThreadAutoLock lock(&m_InstanceLock);
    mp_string strDbFile = CPath::GetInstance().GetDbFilePath(AGENT_DB_FILE);
    return m_db.ExecSql(strDbFile, strSql, dpl);
}

mp_int32 CDB::QueryTable(
    const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    CThreadAutoLock lock(&m_InstanceLock);
    mp_string strDbFile = CPath::GetInstance().GetDbFilePath(AGENT_DB_FILE);
    return m_db.QueryTable(strDbFile, strSql, dpl, readBuff, iRowCount, iColCount);
}

mp_int32 CDB::BeginTrans()
{
    CThreadAutoLock lock(&m_InstanceLock);
    mp_string strDbFile = CPath::GetInstance().GetDbFilePath(AGENT_DB_FILE);
    return m_db.BeginTrans(strDbFile);
}

mp_int32 CDB::RollbackTrans()
{
    CThreadAutoLock lock(&m_InstanceLock);
    return m_db.RollbackTrans();
}

mp_int32 CDB::CommitTrans()
{
    CThreadAutoLock lock(&m_InstanceLock);
    return m_db.CommitTrans();
}

mp_int32 DWSDB::ExecSql(const mp_string &dbFile, const mp_string& strSql, DbParamStream& dpl)
{
    CThreadAutoLock lock(&m_InstanceLock);
    return m_db.ExecSql(dbFile, strSql, dpl);
}

mp_int32 DWSDB::QueryTable(const mp_string &dbFile,
    const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    CThreadAutoLock lock(&m_InstanceLock);
    return m_db.QueryTable(dbFile, strSql, dpl, readBuff, iRowCount, iColCount);
}

mp_int32 DWSDB::ExecSqlNoLock(const mp_string &dbFile, const mp_string& strSql, DbParamStream& dpl)
{
    return m_db.ExecSql(dbFile, strSql, dpl);
}

mp_int32 DWSDB::BeginTrans(const mp_string &dbFile)
{
    return m_db.BeginTrans(dbFile);
}

mp_int32 DWSDB::RollbackTrans()
{
    return m_db.RollbackTrans();
}

mp_int32 DWSDB::CommitTrans()
{
    return m_db.CommitTrans();
}

/* ------------------------------------------------------------
Function Name:operator>>
Description  :>>操作符
Others       :------------------------------------------------------------- */
DbParam DbParamStream::operator>>(DbParam& param)
{
    if (Empty()) {
        COMMLOG(OS_LOG_WARN, "%s", "There has no param.");
        return param;
    }
    param = m_ParamList.front();
    m_ParamList.pop_front();
    return param;
}

DbParam::DbParam(mp_int32 value)
{
    m_type = DB_PARAM_TYPE_INT32;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_int64 value)
{
    m_type = DB_PARAM_TYPE_INT64;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_uint32 value)
{
    m_type = DB_PARAM_TYPE_UINT32;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_uint64 value)
{
    m_type = DB_PARAM_TYPE_UINT64;
    IntToString(value, m_value);
}
