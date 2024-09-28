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
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskContextTest.h"
#include "taskmanager/TaskManager.h"
mp_int32 GetJsonStringTest(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 GetJsonUInt64Test(const Json::Value& jsValue, const mp_string& strKey, mp_uint64& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubCConfigXmlParserGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_void LogTest() {}
#define DoLogTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32Return); \
} while (0)

#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CJsonUtils, GetJsonString), GetJsonStringTest); \
} while (0)

#define DoGetJsonUInt64Test() do { \
    stub.set(ADDR(CJsonUtils, GetJsonUInt64), GetJsonStringTest); \
} while (0)


TEST_F(TaskContextTest, SetJsonValueTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    Json::Value jsonVal;
    jsonVal["aa"] = 1;
    context->SetJsonValue(taskID, strKey, jsonVal);
    context->SetJsonValue(taskID, strKey, jsonVal);
}

TEST_F(TaskContextTest, SetValueStringTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_string strValue = "bbb";
    context->m_taskContextJsonDate.clear();
    context->SetValueString(taskID, strKey, strValue);
    context->SetValueString(taskID, strKey, strValue);
}


TEST_F(TaskContextTest, SetValueInt32Test)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_int32 iValue = 222;
    context->SetValueInt32(taskID, strKey, iValue);
    context->m_taskContextJsonDate.clear();
    context->SetValueInt32(taskID, strKey, iValue);
}

TEST_F(TaskContextTest, SetValueUInt32Test)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_uint32 iValue = 222;
    context->m_taskContextJsonDate.clear();
    context->SetValueUInt32(taskID, strKey, iValue);
    context->SetValueUInt32(taskID, strKey, iValue);
}

TEST_F(TaskContextTest, GetValueStringTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_string iValue = "bbb";
    context->m_taskContextJsonDate.clear();
    context->GetValueString(taskID, strKey, iValue);
    context->GetValueString(taskID, strKey, iValue);
}

TEST_F(TaskContextTest, GetValueInt32Test)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_int32 iValue = 22;
    context->GetValueInt32(taskID, strKey, iValue);
    context->GetValueInt32(taskID, strKey, iValue);
}

TEST_F(TaskContextTest, GetValueUInt32Test)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_uint32 iValue = 22;
    context->GetValueUInt32(taskID, strKey, iValue);
    context->GetValueUInt32(taskID, strKey, iValue);
}

TEST_F(TaskContextTest, GetValueVectorTest)
{
    DoGetJsonUInt64Test();
    StubClogToVoidLogNullPointReference();
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    std::vector<mp_string> vecValue;
    context->GetValueVector(taskID, strKey, vecValue);
    context->GetValueVector(taskID, strKey, vecValue);
}

TEST_F(TaskContextTest, GetValueJsonTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    Json::Value jsValue = "bbb";
    context->m_taskContextJsonDate.clear();
    context->GetValueJson(taskID, strKey, jsValue);
    context->GetValueJson(taskID, strKey, jsValue);
}

TEST_F(TaskContextTest, GetValueStringOptionTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_string strValue = "bbb";
    context->m_taskContextJsonDate.clear();
    context->GetValueStringOption(taskID, strKey, strValue);
    context->GetValueStringOption(taskID, strKey, strValue);
}

TEST_F(TaskContextTest, GetValueInt32OptionTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_int32 strValue = 222;
    context->m_taskContextJsonDate.clear();
    context->GetValueInt32Option(taskID, strKey, strValue);
    context->GetValueInt32Option(taskID, strKey, strValue);
}

TEST_F(TaskContextTest, GetValueUInt32OptionTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    mp_uint32 strValue = 222;
    context->m_taskContextJsonDate.clear();
    context->GetValueUInt32Option(taskID, strKey, strValue);
    context->GetValueUInt32Option(taskID, strKey, strValue);
}

TEST_F(TaskContextTest, GetValueVectorOptionTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID = "bbb";
    mp_string strKey = "bbb";
    std::vector<mp_string> strValue;
    context->m_taskContextJsonDate.clear();
    context->GetValueVectorOption(taskID, strKey, strValue);
    context->GetValueVectorOption(taskID, strKey, strValue);
}

TEST_F(TaskContextTest, RemoveTaskContextTest)
{
    DoLogTest();
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    TaskContext* context = TaskContext::GetInstance();
    mp_string taskID;
    context->RemoveTaskContext(taskID);
}
