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
#include "fuzz/oraclenative/FuzzOracleNative.h"
#include "secodeFuzz.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "plugins/oraclenative/OracleNativeBackupPlugin.h"
#include "apps/oraclenative/TaskStepOracleNativeNasMedia.h"
#include "apps/oraclenative/TaskStepCheckDBOpen.h"
#include "apps/oraclenative/TaskStepOracleNativeBackup.h"
#include "apps/oraclenative/TaskStepCheckDBClose.h"
#include "apps/oraclenative/TaskStepOracleNativeRestore.h"
#include "apps/oraclenative/TaskStepOracleNativeLiveMount.h"
#include "apps/oraclenative/TaskStepOracleNativeCLiveMount.h"
#include "apps/oraclenative/TaskStepOracleNativeInstRestore.h"
#include "apps/oraclenative/TaskStepOracleNativeMoveDBF.h"
#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"
#include "taskmanager/TaskContext.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"
using namespace std;

namespace {
char dt_fuzz_string_init[] = "1234";
int dt_fuzz_int_init = 1;
char dt_fuzz_ipv4_init[4] = {0,0,0,0};
int g_fuzz_index = 0;

mp_string DT_GetString_ParamValid(mp_int32 maxLen)
{
    mp_string strTmp = DT_SetGetString(&g_Element[g_fuzz_index++], 5, maxLen, dt_fuzz_string_init);
    if ((CheckParamValid(strTmp) != MP_SUCCESS)) {
        strTmp = "param";
    }
    return strTmp;
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    if (iCommandID == (mp_int32)ROOT_COMMAND_SCRIPT_QUERYBACKUPLEVEL) {
        pvecResult->push_back("level;1");
    }
    return MP_SUCCESS;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 100000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32_BackupData(mp_void* pThis,
    const mp_string& taskID, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 1;
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32_BackupLog(mp_void* pThis,
    const mp_string& taskID, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 1;
    return MP_SUCCESS;
}
}

TEST_F(FuzzOracleNative, GetDBStorInfo)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    m_stub.set(ADDR(OracleNativeBackup, GetDBStorInfo), StubSUCCESS);
    OracleNativeBackupPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_GetDBStorInfo";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value reqBody;
        reqBody[PARAM_KEY_DBNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        reqBody[PARAM_KEY_INSTNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        reqBody[PARAM_KEY_DBUSER] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        reqBody[PARAM_KEY_DBPWD] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        reqBody[PARAM_KEY_ASMINST] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        reqBody[PARAM_KEY_ASMUSER] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        reqBody[PARAM_KEY_ASMPWD] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] = reqBody;
        CDppMessage reqMsg;
        reqMsg.SetMsgBody(reqBodyParams);
        CDppMessage rspMsg;
        EXPECT_EQ(MP_SUCCESS, plugObj.GetDBStorInfo(reqMsg, rspMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, PrepareMedia)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepOracleNativeNasMedia step("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_PrepareMedia";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
        jStorage["authUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jStorage["authKey"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        for (int i = 0; i < 20; ++i) {
            jStorage["dataOwnerIps"].append(DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING));
            jStorage["dataOtherIps"].append(DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING));
            jStorage["logOwnerIps"].append(DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING));
            jStorage["logOtherIps"].append(DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING));
        }
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        Json::Value reqBody;
        reqBody[keyTaskType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_TASKTYPE);
        reqBody[keyHostRole] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_HOSTROLE);
        reqBody[keyStorage] = jStorage;
        reqBody[keyDbInfos] = jDbInfo;
        EXPECT_EQ(MP_SUCCESS, step.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, QueryBackupLevel)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    m_stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleNativeBackupPlugin plugObj;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_QueryBackupLevel";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value reqBody;
        reqBody[KEY_TASKID] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        reqBody["level"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_LEVEL);
        reqBody["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        reqBody[PARAM_KEY_DBNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        reqBody[PARAM_KEY_INSTNAME] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        reqBody[PARAM_KEY_DBUSER] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        reqBody[PARAM_KEY_DBPWD] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        reqBody[PARAM_KEY_ASMINST] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        reqBody[PARAM_KEY_ASMUSER] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        reqBody[PARAM_KEY_ASMPWD] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] = reqBody;
        CDppMessage reqMsg;
        reqMsg.SetMsgBody(reqBodyParams);
        CDppMessage rspMsg;
        EXPECT_EQ(MP_SUCCESS, plugObj.QueryBackupLevel(reqMsg, rspMsg));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, BackupDataFile)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(TaskContext, GetValueInt32), StubGetValueInt32_BackupData);
    TaskStepCheckDBOpen stepCheckDbOpen("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeBackup stepBackup("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_BackupDataFile";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);

        Json::Value jDbParam;
        jDbParam[g_EncAlgo] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[g_EncKey] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyChannel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_CHANNEL);
        jDbParam[keyLimitSpeed] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyLevel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_LEVEL);
        jDbParam[keyArchiveLogKeepDays] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
        reqBody[keyDbParams] = jDbParam;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbOpen.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepBackup.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, BackupLogFile)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(TaskContext, GetValueInt32), StubGetValueInt32_BackupLog);
    TaskStepCheckDBOpen stepCheckDbOpen("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeBackup stepBackup("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_BackupLogFile";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);

        Json::Value jDbParam;
        jDbParam[g_EncAlgo] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[g_EncKey] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyChannel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_CHANNEL);
        jDbParam[keyLimitSpeed] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyTruncateLog] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, 1);

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
        reqBody[keyDbParams] = jDbParam;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbOpen.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepBackup.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, RestoreOracleDB)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepCheckDBClose stepCheckDbClose("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeRestore stepRestore("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_RestoreOracleDB";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);

        Json::Value jDbParam;
        jDbParam[keyChannel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_CHANNEL);
        jDbParam[keyPitTime] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyPitScn] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyRecoverOrder] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[keyRecoverTarget] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[keyRecoverPath] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyRecoverNum] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[g_EncAlgo] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[g_EncKey] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyRestoreBy] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        Json::Value jpFile;
        jpFile[keyPfile] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyPfileParams] = jpFile;

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
        reqBody[keyDbParams] = jDbParam;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbClose.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepRestore.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, LiveMountOracleDB)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepCheckDBClose stepCheckDbClose("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeLiveMount stepLivemount("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_LiveMountOracleDB";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);

        Json::Value jDbParam;
        jDbParam[keyChannel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_CHANNEL);
        jDbParam[keyStartDB] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, 1);
        jDbParam[keyRecoverOrder] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[keyRecoverNum] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[g_EncAlgo] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[g_EncKey] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        Json::Value jpFile;
        jpFile[keyPfile] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyPfileParams] = jpFile;

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
        reqBody[keyDbParams] = jDbParam;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbClose.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepLivemount.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, CancelLiveMountOracleDB)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepOracleNativeCLiveMount stepCancelLivemount("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeDismount stepDismount("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_CancelLiveMountOracleDB";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
        jStorage["authUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jStorage["authKey"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
         reqBody[keyTaskType] = *(s32 *)DT_SetGetNumberRange(
             &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_TASKTYPE);

        EXPECT_EQ(MP_SUCCESS, stepCancelLivemount.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepDismount.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, InstanceRestore)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepCheckDBClose stepCheckDbClose("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeInstRestore stepInstanceRestore("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeMoveDBF stepMoveDBF("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_InstanceRestore";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);

        Json::Value jDbParam;
        jDbParam[keyChannel] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_CHANNEL);
        jDbParam[keyPitTime] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyPitScn] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INT32);
        jDbParam[keyRecoverOrder] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[keyRecoverTarget] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[keyRecoverPath] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyRecoverNum] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        jDbParam[g_EncAlgo] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[g_EncKey] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyRestoreBy] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_INTGENERAL);
        Json::Value jpFile;
        jpFile[keyPfile] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbParam[keyPfileParams] = jpFile;

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody[keyStorInfo] = jStorage;
        reqBody[keyDbParams] = jDbParam;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbClose.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepInstanceRestore.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepMoveDBF.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, ExpireCopy)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepCheckDBOpen stepCheckDbOpen("id", "taskId", "name", 0, 0);
    TaskStepOracleNativeExpireCopy stepExpire("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_ExpireCopy";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jCopyIngo;
        jCopyIngo["logmaxscn"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);

        Json::Value reqBody;
        reqBody["dbInfo"] = jDbInfo;
        reqBody["copyInfo"] = jCopyIngo;

        EXPECT_EQ(MP_SUCCESS, stepCheckDbOpen.Init(reqBody));
        EXPECT_EQ(MP_SUCCESS, stepExpire.Init(reqBody));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzOracleNative, DisMountMedium)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    TaskStepOracleNativeDismount step("id", "taskId", "name", 0, 0);
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzOracleNative_DisMountMedium";
    DT_FUZZ_START(0, 100000, strDtFuzzName, 0)
    {
        g_fuzz_index = 0;
        Json::Value jDbInfo;
        jDbInfo["dbName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBNAME);
        jDbInfo["dbUUID"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_UUID);
        jDbInfo["instName"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["dbUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["dbPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["ASMInstance"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_DBINSTANCENAME);
        jDbInfo["ASMUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_NSERNAME);
        jDbInfo["ASMPwd"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jDbInfo["dbType"] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_DBTYPE);

        Json::Value jStorage;
        jStorage[keyStorType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
        jStorage["authUser"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);
        jStorage["authKey"] = DT_GetString_ParamValid(ORACLE_PLUGIN_MAX_STRING);

        Json::Value reqBody;
        reqBody[keyTaskType] = *(s32 *)DT_SetGetNumberRange(
            &g_Element[g_fuzz_index++], dt_fuzz_int_init, 0, ORACLE_PLUGIN_MAX_TASKTYPE);
        reqBody[keyStorage] = jStorage;
        reqBody[keyDbInfos] = jDbInfo;
        EXPECT_EQ(MP_SUCCESS, step.Init(reqBody));
    }
    DT_FUZZ_END()
}