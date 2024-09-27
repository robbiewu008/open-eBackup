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
#include "common/JsonUtilsTest.h"
#include <algorithm>

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(JsonUtilsTest, GetJsonBool)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string strKey = "data";
    mp_bool value;

    iRet = work.GetJsonBool(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    hostMsg["data"] = 1234;
    iRet = work.GetJsonBool(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    hostMsg["data"] = true;
    iRet = work.GetJsonBool(hostMsg, strKey, value);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(JsonUtilsTest, GetJsonInt64)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string strKey = "data";
    mp_int64 value;

    iRet = work.GetJsonInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    hostMsg["data"] = 2015;
    iRet = work.GetJsonInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(JsonUtilsTest, GetJsonUInt64)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string strKey = "data";
    mp_uint64 value;

    iRet = work.GetJsonUInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    hostMsg["data"] = 2015;
    iRet = work.GetJsonUInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(JsonUtilsTest, GetJsonArrayInt32)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string strKey = "data";
    vector<mp_int32> value;

    iRet = work.GetJsonArrayInt32(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    mp_string strArray = "{\"data\":[2015,2016,2017]}";
    iRet = work.ConvertStringtoJson(strArray, hostMsg);
    EXPECT_EQ(iRet, MP_SUCCESS);

    iRet = work.GetJsonArrayInt32(hostMsg, strKey, value);
    EXPECT_EQ(iRet, MP_SUCCESS);

    Json::Value jsValue, a, b;
    a["id"] = "idstr1";
    b["id"] = "idstr2";
    jsValue["data"].append(a);
    jsValue["data"].append(b);
    iRet = work.GetJsonArrayInt32(jsValue, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetJsonArrayInt64)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string strKey = "data";
    vector<mp_int64> value;

    iRet = work.GetJsonArrayInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    mp_string strArray = "{\"data\":[2015,2016,2017]}";
    iRet = work.ConvertStringtoJson(strArray, hostMsg);
    EXPECT_EQ(iRet, MP_SUCCESS);

    iRet = work.GetJsonArrayInt64(hostMsg, strKey, value);
    EXPECT_EQ(iRet, MP_SUCCESS);

    Json::Value jsValue, a, b, jsv, jsValuej;
    a["id"] = "str1";
    b["id"] = "str2";
    mp_string key = "key";
    jsValue["key"].append(a);
    jsValue["key"].append(b);
    jsValuej["key"].append("stringstr1");
    jsValuej["key"].append("stringstr2");
    jsv.append(a);
    jsv.append(b);
    iRet = work.GetJsonArrayInt64(jsValue, key, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    vector<mp_string> vecVal;
    iRet = work.GetJsonArrayString(jsv, key, vecVal);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    iRet = work.GetJsonArrayString(jsValue, key, vecVal);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    vector<Json::Value> vecValuee;
    iRet = work.GetJsonArrayJson(jsv, key, vecValuee);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    iRet = work.GetJsonArrayJson(jsValuej, key, vecValuee);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    vector<mp_int32> vecValue3;
    iRet = work.GetArrayInt32(jsValue, vecValue3);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    iRet = work.GetArrayInt32(jsv, vecValue3);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    vector<mp_int64> vecValue4;
    iRet = work.GetArrayInt64(jsValue, vecValue4);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    iRet = work.GetArrayInt64(jsv, vecValue4);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetArrayInt32)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    vector<mp_int32> value;

    iRet = work.GetArrayInt32(hostMsg, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetArrayInt64)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg[3];
    vector<mp_int64> value;

    iRet = work.GetArrayInt64(hostMsg, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetArrayString)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    vector<mp_string> value;

    iRet = work.GetArrayString(hostMsg, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    Json::Value keyJson2, a, b;
    a["id"] = "id1";
    b["id"] = "id2";
    keyJson2.append(a);
    keyJson2.append(b);
    mp_int32 ret = work.GetArrayString(keyJson2, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetArrayJson)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    vector<Json::Value> value;

    iRet = work.GetArrayJson(hostMsg, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

TEST_F(JsonUtilsTest, GetJsonKeyString)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CJsonUtils work;
    mp_int32 iRet;
    Json::Value hostMsg;
    mp_string key = "paras";
    mp_string value;

    iRet = work.GetJsonKeyString(hostMsg, key, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    Json::Value keyJson;
    keyJson["key"] = "chengdu";
    hostMsg["paras"] = keyJson;

    iRet = work.GetJsonKeyString(hostMsg, key, value);
    EXPECT_EQ(iRet, MP_SUCCESS);

    Json::Value a, keyJson2, hostMsg2, keyJson3;
    a["id"] = "id1";
    keyJson2["key"] = a;
    hostMsg2["paras"] = keyJson2;
    iRet = work.GetJsonKeyString(hostMsg2, key, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    iRet = work.GetJsonString(hostMsg2, key, value);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    
    keyJson3.append("abc");
    keyJson3.append("abcde");
    mp_string bvalue;
    mp_bool bvalue2 {false};
    iRet = work.GetJsonString(keyJson3, key, bvalue);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
    iRet = work.GetJsonBool(keyJson3, key, bvalue2);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}


TEST_F(JsonUtilsTest, GetArrayStringWithoutBraces)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value keyJson;
    mp_string str1("test1");
    mp_string str2("test2");
    keyJson.append(str1);
    keyJson.append(str2);

    std::vector<mp_string> vecValue;
    CJsonUtils::GetArrayStringWithoutBraces(keyJson, vecValue);
    {
        auto pos = std::find(vecValue.begin(), vecValue.end(), str1);
        EXPECT_NE(pos, vecValue.end());
    }
    {
        auto pos = std::find(vecValue.begin(), vecValue.end(), str2);
        EXPECT_NE(pos, vecValue.end());
    }

    Json::Value keyJson2, keyJson3;
    mp_int32 a = 2, b = 3, ret;
    keyJson2["id"] = "idstr";
    ret = CJsonUtils::GetArrayStringWithoutBraces(keyJson2, vecValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);
    
    keyJson3.append(a);
    keyJson3.append(b);
    ret = CJsonUtils::GetArrayStringWithoutBraces(keyJson3, vecValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);
}
TEST_F(JsonUtilsTest, GetArrayInt32WithoutBraces)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value keyJson;
    mp_int32 value1 = 10;
    mp_int32 value2 = 30;
    keyJson.append(value1);
    keyJson.append(value2);

    std::vector<mp_int32> vecValue;
    CJsonUtils::GetArrayInt32WithoutBraces(keyJson, vecValue);
    {
        auto pos = std::find(vecValue.begin(), vecValue.end(), value1);
        EXPECT_NE(pos, vecValue.end());
    }
    {
        auto pos = std::find(vecValue.begin(), vecValue.end(), value2);
        EXPECT_NE(pos, vecValue.end());
    }

    Json::Value keyJson2, keyJson3;
    mp_int32 ret;
    keyJson2["id"] = "keyJson2";
    ret = CJsonUtils::GetArrayInt32WithoutBraces(keyJson2, vecValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);

    keyJson3.append("str1");
    keyJson3.append("str2");
    ret = CJsonUtils::GetArrayInt32WithoutBraces(keyJson3, vecValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);

    mp_string strKey = "key";
    mp_int32 iValue;
    mp_int64 sValue;
    mp_uint64 vValue;
    ret = CJsonUtils::GetJsonInt32(keyJson3, strKey, iValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);

    Json::Value keyJson4;
    keyJson4["key"] = "stringstr";
    ret = CJsonUtils::GetJsonInt32(keyJson4, strKey, iValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);

    ret = CJsonUtils::GetJsonInt64(keyJson4, strKey, sValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);
    ret = CJsonUtils::GetJsonUInt64(keyJson4, strKey, vValue);
    EXPECT_EQ(ret, ERROR_COMMON_INVALID_PARAM);
}
