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
#include "plugins/oraclenative/OracleNativePluginTest.h"
#include "plugins/oraclenative/OracleNativeBackupPlugin.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"
using namespace std;

static char str11[30];
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 GetDBStorInfoS(const oracle_db_info_t& stDBInfo, Json::Value& dbInfo)
{
    return MP_SUCCESS;
}
mp_int32 GetManageBodyTest(Json::Value& dppBody)
{
    return MP_SUCCESS;
}
static bool  stub_return_bool_StopResouceGroup(mp_string keyStorage)
{
    return MP_TRUE;
}
mp_int32 GetDBStorInfoTest(const oracle_db_info_t& stDBInfo, Json::Value& dbInfo)
{
    return MP_FAILED;
}

mp_int32 QueryBackupLevelTest(const Json::Value& reqBody, Json::Value& rspBody)
{
    return MP_SUCCESS;
}

mp_int32 BackupDataTest(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    return MP_SUCCESS;
}

mp_char* GetBufferTest()
{
    return str11;
}

mp_int32 StopTaskTest(const mp_string& msgBody, mp_string& taskId)
{
    return MP_SUCCESS;
}
mp_int32 QueryHostRoleInClusterTest(const oracle_db_info_t& stDBinfo, vector<mp_string>& storHosts, vector<mp_string>& dbHosts)
{
    storHosts.push_back("aaa");
    dbHosts.push_back("aaa");
    return MP_SUCCESS;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

TEST_F(COracleNativeBackupPluginTest, DoAction)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;

    iRet = plugObj.DoAction(req, rsp);

    Json::Value reqMsgVal;
    Json::Value reqBody;
    reqBody["dbName"] = "db1";
    reqMsgVal["body"] = reqBody;
    reqMsgVal["cmd"] = 1;
    req.InitMsgHead(MSG_DATA_TYPE_MANAGE, 0, 0);
    req.SetMsgBody(reqMsgVal);

    iRet = plugObj.DoAction(req, rsp);
}

TEST_F(COracleNativeBackupPluginTest, InitTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    std::vector<mp_uint32> cmds;
    plugObj.Init(cmds);
}

TEST_F(COracleNativeBackupPluginTest, GetDBStorInfoTest1)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    std::vector<mp_uint32> cmds;
    Json::Value val;
    val["dbName"] = "aaaa";
    val["instName"] = "bbbb";
    val["dbUser"] = "bbbb";
    val["dbPwd"] = "bbbb";
    val["dbType"] = "dbType";
    val["ASMInstance"] = "dbType";
    val["ASMUser"] = "ASMUser";
    val["ASMPwd"] = "ASMPwd";
    // stub.set(ADDR(CDppMessage, GetManageBody), GetManageBodyTest);
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    plugObj.GetDBStorInfo(req, rsp);
    req.manageBody["body"] = val;
    plugObj.GetDBStorInfo(req, rsp);
    stub.set(ADDR(OracleNativeBackup, GetDBStorInfo), GetDBStorInfoS);
    plugObj.GetDBStorInfo(req, rsp);

    stub.reset(ADDR(CDppMessage, GetManageBody));
    stub.reset(ADDR(OracleNativeBackup, GetDBStorInfo));
}

TEST_F(COracleNativeBackupPluginTest, QueryBackupLevelTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetManageBody), GetManageBodyTest);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, plugObj.QueryBackupLevel(req, rsp));
    Json::Value val;
    val["dbName"] = "aaaa";
    val["instName"] = "bbbb";
    val["dbUser"] = "bbbb";
    val["dbPwd"] = "bbbb";
    val["dbType"] = "dbType";
    val["ASMInstance"] = "dbType";
    val["ASMUser"] = "ASMUser";
    val["ASMPwd"] = "ASMPwd";
    req.manageBody["body"] = val;
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    plugObj.QueryBackupLevel(req, rsp);
    stub.set(ADDR(OracleNativeBackup, QueryBackupLevel), QueryBackupLevelTest);
    plugObj.QueryBackupLevel(req, rsp);

    stub.reset(ADDR(CDppMessage, GetManageBody));
    stub.reset(ADDR(OracleNativeBackup, QueryBackupLevel));
}

TEST_F(COracleNativeBackupPluginTest, BackupDataFileTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.BackupDataFile(req, rsp);
    stub.set(ADDR(OracleNativeBackup, BackupData), BackupDataTest);
    plugObj.BackupDataFile(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, BackupData));
}

TEST_F(COracleNativeBackupPluginTest, BackupLogFileTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.BackupLogFile(req, rsp);
    stub.set(ADDR(OracleNativeBackup, BackupLog), BackupDataTest);

    plugObj.BackupLogFile(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, BackupLog));
}

TEST_F(COracleNativeBackupPluginTest, RestoreOracleDBTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.RestoreOracleDB(req, rsp);
    stub.set(ADDR(OracleNativeBackup, RestoreDB), BackupDataTest);
    plugObj.RestoreOracleDB(req, rsp);

    stub.reset(ADDR(OracleNativeBackup, RestoreDB));
}

TEST_F(COracleNativeBackupPluginTest, LiveMountOracleDBTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.LiveMountOracleDB(req, rsp);
    stub.set(ADDR(OracleNativeBackup, LiveMount), BackupDataTest);
    plugObj.LiveMountOracleDB(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, LiveMount));
}

TEST_F(COracleNativeBackupPluginTest, CancelLiveMountOracleDBTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.CancelLiveMountOracleDB(req, rsp);
    stub.set(ADDR(OracleNativeBackup, CancelLiveMount), BackupDataTest);
    plugObj.CancelLiveMountOracleDB(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, CancelLiveMount));
}

TEST_F(COracleNativeBackupPluginTest, InstanceRestoreTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.InstanceRestore(req, rsp);
    stub.set(ADDR(OracleNativeBackup, InstanceRestore), BackupDataTest);
    plugObj.InstanceRestore(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, InstanceRestore));
}

TEST_F(COracleNativeBackupPluginTest, ExpireCopyTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.ExpireCopy(req, rsp);
    stub.set(ADDR(OracleNativeBackup, ExpireCopy), BackupDataTest);
    plugObj.ExpireCopy(req, rsp);
    stub.reset(ADDR(OracleNativeBackup, ExpireCopy));
}

TEST_F(COracleNativeBackupPluginTest, DisMountMediumTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetManageBody), GetManageBodyTest);
    plugObj.DisMountMedium(req, rsp);
    Json::Value val;
    val["dbName"] = "aaaa";
    val["instName"] = "bbbb";
    val["dbUser"] = "bbbb";
    val["dbPwd"] = "bbbb";
    val["dbType"] = "dbType";
    val["ASMInstance"] = "dbType";
    val["ASMUser"] = "ASMUser";
    val["ASMPwd"] = "ASMPwd";
    req.manageBody["body"] = val;
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    plugObj.DisMountMedium(req, rsp);
    stub.reset(ADDR(CDppMessage, GetManageBody));
}

TEST_F(COracleNativeBackupPluginTest, StopTaskTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetBuffer), GetBufferTest);
    plugObj.StopTask(req, rsp);
    stub.set(ADDR(OracleNativeBackup, StopTask), StopTaskTest);
    plugObj.StopTask(req, rsp);
}

TEST_F(COracleNativeBackupPluginTest, PrepareMediaTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetManageBody), GetManageBodyTest);
    plugObj.PrepareMedia(req, rsp);
    Json::Value val;
    val["dbName"] = "aaaa";
    val["instName"] = "bbbb";
    val["dbUser"] = "bbbb";
    val["dbPwd"] = "bbbb";
    val["dbType"] = "dbType";
    val["ASMInstance"] = "dbType";
    val["ASMUser"] = "ASMUser";
    val["ASMPwd"] = "ASMPwd";
    req.manageBody["body"] = val;
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    plugObj.PrepareMedia(req, rsp);
    stub.set(ADDR(Json::Value, isArray), stub_return_bool_StopResouceGroup);
}

TEST_F(COracleNativeBackupPluginTest, QueryHostRoleInClusterTest)
{
    stub11.set(&CLogger::Log, StubCLoggerLog);
    Stub stub;
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    CDppMessage req;
    CDppMessage rsp;
    mp_int32 iRet = MP_SUCCESS;
    OracleNativeBackupPlugin plugObj;
    stub.set(ADDR(CDppMessage, GetManageBody), GetManageBodyTest);
    plugObj.QueryHostRoleInCluster(req, rsp);
    Json::Value val;
    val["dbName"] = "aaaa";
    val["instName"] = "bbbb";
    val["dbUser"] = "bbbb";
    val["dbPwd"] = "bbbb";
    val["dbType"] = "dbType";
    val["ASMInstance"] = "dbType";
    val["ASMUser"] = "ASMUser";
    val["ASMPwd"] = "ASMPwd";
    req.manageBody["body"] = val;
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    plugObj.QueryHostRoleInCluster(req, rsp);
    stub.set(ADDR(Oracle, QueryHostRoleInCluster), QueryHostRoleInClusterTest);
    plugObj.QueryHostRoleInCluster(req, rsp);
}