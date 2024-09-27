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
#include "common/DbTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubCDBConnect(mp_bool isTrans)
{
    return MP_SUCCESS;
}

sqlite3_stmt* StubCDBSqlPrepare(mp_string sql)
{
    return NULL;
}

TEST_F(DbTest, BeginTrans)
{
    mp_int32 iRet;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = CDB::GetInstance().BeginTrans();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, RollbackTrans)
{
    mp_int32 iRet;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = CDB::GetInstance().RollbackTrans();
    EXPECT_NE(iRet, MP_SUCCESS);
}

TEST_F(DbTest, CommitTrans)
{
    mp_int32 iRet;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = CDB::GetInstance().CommitTrans();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, SqlPrepare)
{
    DB work;
    sqlite3_stmt* iRet = NULL;
    mp_string sqlStr = "test";

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.SqlPrepare(sqlStr);
}

TEST_F(DbTest, SqlBind)
{
    DB work;
    mp_int32 iRet;
    mp_string sqlStr = "test";
    sqlite3_stmt* stmt = NULL;
    DbParamStream dps;
    DbParam element1,element2,element3;
    element1.m_type = DB_PARAM_TYPE_INT32;
    element2.m_type = DB_PARAM_TYPE_UINT32;
    element3.m_type = DB_PARAM_TYPE_STRING;

    stub.set(&CLogger::Log, StubCLoggerLog);
    /* dps 为空 */
    iRet = work.SqlBind(*stmt, dps);
    EXPECT_EQ(iRet, MP_SUCCESS);

    dps << element1;
    iRet = work.SqlBind(*stmt, dps);
    EXPECT_EQ(iRet, MP_FAILED);

    dps.Clear();
    dps << element2;
    iRet = work.SqlBind(*stmt, dps);
    EXPECT_EQ(iRet, MP_FAILED);

    dps.Clear();
    dps << element3;
    iRet = work.SqlBind(*stmt, dps);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, SqlExecute)
{
    DB work;
    mp_int32 iRet;
    mp_string sqlStr = "test";
    sqlite3_stmt* stmt = NULL;

    stub.set(&CLogger::Log, StubCLoggerLog);
    /* dps 为空 */
    iRet = work.SqlExecute(*stmt);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, ExecSql)
{
    mp_int32 iRet;
    mp_string sqlStr = "test";
    DbParamStream dps;
    sqlite3_stmt* stmt = NULL;

    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(DB,Connect), StubCDBConnect);
    iRet = CDB::GetInstance().ExecSql(sqlStr, dps);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, QueryTable)
{
    mp_int32 iRet;
    mp_string sqlStr = "test";
    DbParamStream dps;
    sqlite3_stmt* stmt = NULL;
    DBReader readBuff;
    mp_int32 iRowCount;
    mp_int32 iColCount;

    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(DB,Connect), StubCDBConnect);
    iRet = CDB::GetInstance().QueryTable(sqlStr, dps, readBuff, iRowCount, iColCount);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DbTest, SqlQuery)
{
    DB work;
    mp_int32 iRet;
    mp_string sqlStr = "test";
    DbParamStream dps;
    sqlite3_stmt* stmt = NULL;
    DBReader readBuff;
    mp_int32 iRowCount;
    mp_int32 iColCount;

    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(DB,Connect), StubCDBConnect);
    iRet = work.SqlQuery(*stmt, readBuff, iRowCount, iColCount);
    EXPECT_EQ(iRet, MP_FAILED);
}
