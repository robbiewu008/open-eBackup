/*
 * Copyright (C), 2001-2020, Huawei Tech. Co., Ltd.
 * Description: oracle native backup base class
 * Author: wangguitao w00510599
 * Create: 2020-01-27
 * History:
 *   2020-01-27 initial
 */
#include "apps/oraclenative/TaskStepOracleNativeTest.h"
#include "apps/oraclenative/TaskStepOracleNative.h"
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

};


TEST_F(TaskStepOracleNativeTest, NativeInit)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    oracleNative.Init(param);
    oracleNative.Run();
    oracleNative.Cancel();
    oracleNative.Cancel(param);
    oracleNative.Stop(param);
    Json::Value repParam;
    oracleNative.Update(param);
    oracleNative.Update(param, repParam);
    oracleNative.Finish(param);
    oracleNative.Finish(param, repParam);
    oracleNative.Redo(taskId);

}

TEST_F(TaskStepOracleNativeTest, InitialDBInfoTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    oracleNative.InitialDBInfo(param);
    param["dbInfo"] = "111";
    oracleNative.InitialDBInfo(param);
}

TEST_F(TaskStepOracleNativeTest, InitExtrasParamsTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    oracleNative.InitExtrasParams(param);
    param["hostRole"] = "111";
    oracleNative.InitExtrasParams(param);
}

TEST_F(TaskStepOracleNativeTest, InitStorTypeTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    oracleNative.InitStorType(param);
    param["storage"] = "111";
    // oracleNative.InitStorType(param);
}


TEST_F(TaskStepOracleNativeTest, InitialVolInfoTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    mp_char flag = 1;
    oracleNative.InitialVolInfo(param, flag);
    param["storage"] = "111";
    oracleNative.InitialVolInfo(param, flag);
}

TEST_F(TaskStepOracleNativeTest, JsonArray2VolumeArrTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    std::vector<Json::Value> jsonArr;
    std::vector<BackupVolume> volArr;
    mp_string metaFlag;
    mp_string volumeType;
    jsonArr.push_back(Json::Value());
    oracleNative.JsonArray2VolumeArr(jsonArr, volArr, metaFlag, volumeType);
}

TEST_F(TaskStepOracleNativeTest, CheckVolsValidTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    mp_char flag = 1;
    oracleNative.CheckVolsValid(flag);
}

TEST_F(TaskStepOracleNativeTest, GetDiskListByWWNTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    vector<BackupVolume> vols;
    mp_string diskList;
    oracleNative.GetDiskListByWWN(vols, diskList);
}

TEST_F(TaskStepOracleNativeTest, AnalyseQueryRmanStatusScriptRstTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    vector<mp_string> vecResult;
    mp_int32 speed;
    mp_int32 progress;
    vecResult.push_back("11");
    oracleNative.AnalyseQueryRmanStatusScriptRst(vecResult, speed, progress);
}

TEST_F(TaskStepOracleNativeTest, FillSpeedAndProgressTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    mp_string vecResult;
    mp_int32 speed;
    mp_int32 progress;
    vecResult = "11;11";
    oracleNative.FillSpeedAndProgress(vecResult, speed, progress);
}

TEST_F(TaskStepOracleNativeTest, ConvertStatusTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    mp_string status;
    oracleNative.ConvertStatus(status);
    oracleNative.m_mapRmanTaskStatus["11"] = 11;
    oracleNative.ConvertStatus(status);
}

TEST_F(TaskStepOracleNativeTest, BuildPfileInfoTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNative oracleNative(id, taskId, name, ratio , order);
    oracleNative.BuildPfileInfo();
}