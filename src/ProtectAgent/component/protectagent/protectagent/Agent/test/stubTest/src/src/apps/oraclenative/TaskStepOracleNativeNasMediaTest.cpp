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
#include "apps/oraclenative/TaskStepOracleNativeNasMedia.h"
#include "apps/oraclenative/TaskStepOracleNativeRestore.h"
#include "apps/oraclenative/TaskStepOracleNativeNasMediaTest.h"
#include "host/host.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeNasMediaTest : public testing::Test {
public:
    TaskStepOracleNativeNasMediaTest() { }
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepOracleNativeNasMediaTest::SetUp()
{}

void TaskStepOracleNativeNasMediaTest::TearDown()
{}

void TaskStepOracleNativeNasMediaTest::SetUpTestCase()
{}

void TaskStepOracleNativeNasMediaTest::TearDownTestCase()
{}

using namespace std;
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

mp_int32 InitParaTest(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 MountNasMediaTest(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    vecRst.push_back("/home1/");
    vecRst.push_back("/home2/");
    vecRst.push_back("/home3/");
    return MP_SUCCESS;
}

mp_int32 MountNasMediaTest_FAILED(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    return MP_FAILED;
}

mp_int32 MountDataturboMediaTest(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst, DataturboMountParam &param)
{
    vecRst.push_back("/home1/");
    vecRst.push_back("/home2/");
    vecRst.push_back("/home3/");
    return MP_SUCCESS;    
}

mp_int32 MountDataturboMediaTest_FAILED(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst, DataturboMountParam &param)
{
    return MP_FAILED;    
}

mp_int32 GetHostSN_Stub(mp_string& strSN)
{
    strSN="12345";
    return MP_SUCCESS;
}


mp_int32 UpdateParamTest(mp_void* pThis, const mp_string& paramID, const mp_string& paramKey, const mp_string& paramValue)
{
    return MP_SUCCESS;
}
};

TEST_F(TaskStepOracleNativeNasMediaTest, nasMediaInitTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    nasMedia.Init(param);
    stub.set(ADDR(TaskStepOracleNativeNasMedia, InitPara), InitParaTest);
    nasMedia.Init(param);
    stub.reset(ADDR(TaskStepOracleNativeNasMedia, InitPara));
}

TEST_F(TaskStepOracleNativeNasMediaTest, nasMediaRun)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia), MountNasMediaTest);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest);
    stub.set(ADDR(TaskStepPrepareNasMedia, UpdateParam), UpdateParamTest);
    nasMedia.Run();
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountNasMedia));
    stub.reset(ADDR(TaskStepPrepareNasMedia, UpdateParam));
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia));
}

TEST_F(TaskStepOracleNativeNasMediaTest, nasMediaBuildScriptParam)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    mp_string strParam;
    // nasMedia.BuildScriptParam(strParam);
}

TEST_F(TaskStepOracleNativeNasMediaTest, nasMediaStop)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    nasMedia.Stop(param);
}

TEST_F(TaskStepOracleNativeNasMediaTest, nasMediaCancel)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    nasMedia.Cancel();
    nasMedia.Update(param);
    nasMedia.Finish(param);
}

TEST_F(TaskStepOracleNativeNasMediaTest, restoreInitTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore nativeRestore(id, taskId, name, ratio , order);
    nativeRestore.Init(param);
}

TEST_F(TaskStepOracleNativeNasMediaTest, restoreRun)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore nativeRestore(id, taskId, name, ratio , order);
    TaskStepOracleNativeNasMedia nativeNasRestore(id, taskId, name, ratio , order);
    stub.set(ADDR(TaskStepOracleNativeNasMedia, MountNasMedia), MountNasMediaTest);
    nativeNasRestore.Run();
    stub.reset(ADDR(TaskStepOracleNativeNasMedia, MountNasMedia));
}

TEST_F(TaskStepOracleNativeNasMediaTest,restoreBuildScriptParam)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore nativeRestore(id, taskId, name, ratio , order);
    mp_string strParam;
    // nativeRestore.BuildScriptParam(strParam);
}

TEST_F(TaskStepOracleNativeNasMediaTest, restoreStop)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore nativeRestore(id, taskId, name, ratio , order);
    nativeRestore.Stop(param);
}

TEST_F(TaskStepOracleNativeNasMediaTest, restoreCancel)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeRestore nativeRestore(id, taskId, name, ratio , order);
    nativeRestore.Cancel();
}

/*
* 用例描述：检验挂载文件系统流程
* 观测点：(1) Dataturbo挂载成功，NFS挂载成功，返回成功
*        (2) Dataturbo挂载失败，NFS挂载成功，返回成功
*        (2) Dataturbo挂载失败，NFS挂载失败，返回失败
*/
TEST_F(TaskStepOracleNativeNasMediaTest, BackupMountStorageNasMedia)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    stub.set(ADDR(CHost, GetHostSN), GetHostSN_Stub);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia), MountNasMediaTest);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest);
    TaskStepOracleNativeNasMedia nasMedia(id, taskId, name, ratio , order);
    std::vector<mp_string> vecRst;
    mp_int32 iRet = nasMedia.MountStorageNasMedia(vecRst);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest_FAILED);
    vecRst.clear();
    iRet = nasMedia.MountStorageNasMedia(vecRst);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia), MountNasMediaTest_FAILED);
    vecRst.clear();
    iRet = nasMedia.MountStorageNasMedia(vecRst);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset(ADDR(CHost, GetHostSN));
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia));
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountNasMedia));
}
