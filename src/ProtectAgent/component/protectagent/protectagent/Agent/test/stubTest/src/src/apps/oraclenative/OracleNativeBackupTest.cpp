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
#include "apps/oraclenative/OracleNativeBackupTest.h"

namespace {
mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

void QueryMountPointInfoTest(std::map<mp_string, std::set<mp_string>>& mapMountInfo)
{
    std::set<mp_string> setInfo;
    setInfo.insert("192");
    mapMountInfo["192"] = setInfo;
}
mp_int32 GetTaskidByReqTest(const mp_string& msgBody, mp_string& taskid)
{
    return MP_SUCCESS;
}
};

TEST_F(COracleNativeBackupTest, GetDBStorInfoGTest)
{
    // DoGetJsonStringTest();
    // OracleNativeBackup backup;
    // oracle_db_info_t stDBInfo;
    // Json::Value dbInfo;
    // backup.GetDBStorInfo(stDBInfo, dbInfo);
    // stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    // backup.GetDBStorInfo(stDBInfo, dbInfo);
    // stub.reset(ADDR(CRootCaller, Exec));
}


TEST_F(COracleNativeBackupTest, CheckMountTestA)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();

    backup->CheckMount();
    // stub.set(ADDR(OracleNativeBackup, QueryMountPointInfo), QueryMountPointInfoTest);
    // backup->CheckMount();
    // stub.reset(ADDR(OracleNativeBackup, QueryMountPointInfo));
}

TEST_F(COracleNativeBackupTest, BackupOracleTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    mp_string msgBody;
    mp_string connIp;
    mp_uint16 connPort;
    mp_int32 backupMode;
    mp_string taskId;
    backup->BackupOracle(msgBody, connIp, connPort, backupMode, taskId);
    stub.set(ADDR(OracleNativeBackup, GetTaskidByReq), GetTaskidByReqTest);
    backup->BackupOracle(msgBody, connIp, connPort, backupMode, taskId);
    stub.reset(ADDR(OracleNativeBackup, GetTaskidByReq));
}


TEST_F(COracleNativeBackupTest, RestoreDBTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    mp_string msgBody;
    mp_string connIp;
    mp_uint16 connPort;
    mp_string taskId;
    backup->RestoreDB(msgBody, connIp, connPort, taskId);
}


TEST_F(COracleNativeBackupTest, ExpireCopyTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    mp_string msgBody;
    mp_string connIp;
    mp_uint16 connPort;
    mp_string taskId;
    backup->ExpireCopy(msgBody, connIp, connPort, taskId);
}

TEST_F(COracleNativeBackupTest, BuildQueryStorageTypeScriptParamTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    oracle_db_info_t stDBInfo;
    mp_string strParam;
    backup->BuildQueryStorageTypeScriptParam(stDBInfo, strParam);
}

TEST_F(COracleNativeBackupTest, AnalyseStorInfoScriptRstTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    vector<mp_string> vecResult;
    Json::Value dbInfo;
    backup->AnalyseStorInfoScriptRst(vecResult, dbInfo);
}

TEST_F(COracleNativeBackupTest, DataCapHandlerTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    vector<mp_string> vecResult;
    Json::Value dbInfo;
    backup->DataCapHandler(vecResult, dbInfo);
}

TEST_F(COracleNativeBackupTest, DBTypeHandlerTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    vector<mp_string> vecResult;
    Json::Value dbInfo;
    backup->DBTypeHandler(vecResult, dbInfo);
}

TEST_F(COracleNativeBackupTest, DBInsLstHandlerTest)
{
    DoGetJsonStringTest();
    OracleNativeBackup *backup = new OracleNativeBackup();
    vector<mp_string> vecResult;
    Json::Value dbInfo;
    backup->DBInsLstHandler(vecResult, dbInfo);
}
*/


