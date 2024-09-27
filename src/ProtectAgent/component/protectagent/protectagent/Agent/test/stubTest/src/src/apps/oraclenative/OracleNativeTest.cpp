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
#include "apps/oraclenative/OracleNativeTest.h"
#include "taskmanager/TaskContext.h"
#include "apps/oraclenative/OracleNativeTaskTest.h"

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

TEST_F(COracleNativeBackupTest, QueryBackupLevel)
{
    DoGetJsonStringTest();
    Json::Value reqBody;
    Json::Value rspBody;
    OracleNativeBackup work;
    mp_int32 iRet;

    reqBody[MANAGECMD_KEY_TASKID] = "4004ddd";
    reqBody["level"] = 1;
    reqBody["dbUUID"] = "40044004ddasd";
    reqBody["dbName"] = "test";
    reqBody["instName"] = "test";
    reqBody["dbUser"] = "test";
    reqBody["dbPwd"] = "4004";
    reqBody["ASMInstance"] = "test";
    reqBody["ASMUser"] = "test";
    reqBody["ASMPwd"] = "4004";

    stub.set(ADDR(CJsonUtils,GetJsonString), StubCJsonUtilsGetJsonString);
    stub.set(ADDR(CJsonUtils,GetJsonInt32), StubCJsonUtilsGetJsonInt32);

    stub.set(ADDR(CRootCaller, Exec), StubCRootCallerExecGetDBType2);
    iRet = work.QueryBackupLevel(reqBody, rspBody);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(COracleNativeBackupTest, PrepareMedia)
{
    DoGetJsonStringTest();
    OracleNativeBackup work;
    mp_int32 iRet;
    mp_string msgBody = "{\"body\" : \"{\"taskId\":\"10024aabb\"}\"}";
    mp_int32 taskType = 1;
    mp_int32 storType = 1;

    Json::Value repsJson;
    iRet = work.PrepareMedia(msgBody, taskType, storType, repsJson);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(OracleNativeBackup,GetTaskidByReq), StubGetTaskidByReq);
    iRet = work.PrepareMedia(msgBody, taskType, storType, repsJson);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(COracleNativeBackupTest, DisMountMedium)
{
    DoGetJsonStringTest();
    OracleNativeBackup work;
    mp_int32 iRet;
    mp_string msgBody = "{\"body\" : \"{\"taskId\":\"10024aabb\"}\"}";
    mp_int32 taskType = 1;
    mp_int32 storType = 1;

    iRet = work.DisMountMedium(msgBody, taskType);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(OracleNativeBackup,GetTaskidByReq), StubGetTaskidByReq);
    iRet = work.DisMountMedium(msgBody, taskType);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(COracleNativeBackupTest, LogCapHandler)
{
    DoGetJsonStringTest();
    OracleNativeBackup work;
    mp_int32 iRet;
    vector<mp_string> storInfos;
    Json::Value dbInfo;
    storInfos.push_back("1");
    storInfos.push_back("2");

    iRet = work.LogCapHandler(storInfos, dbInfo);
    EXPECT_EQ(iRet, ERROR_COMMON_OPER_FAILED);

    storInfos.push_back("3");
    iRet = work.LogCapHandler(storInfos, dbInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
