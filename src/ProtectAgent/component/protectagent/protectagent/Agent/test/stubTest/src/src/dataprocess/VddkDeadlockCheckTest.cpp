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
#include "dataprocess/VddkDeadlockCheckTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubVddkDeadlockCheckGetValueInt32Return);                                                                      \
    } while (0)
namespace {
const std::string MODULE_NAME = "AgentVddkDeadlockCheckService";
const mp_int32 CHECK_PERIOD_IN_SECONDS = 600;
const mp_int32 CHECK_PERIOD_IN_NUMS = 1000;
const mp_int32 MAX_MEMORY_SIZE = (2L * 1024 * 1024);  // KB
const mp_int32 CHECK_DENO = 1000000000;
const mp_int32 INVOKE_TIME_OUT = 7200;
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

// TEST_F(VddkDeadlockCheckTest, ContructDeadlockCheck)
// {
//     VddkDeadlockCheck::DeadlockCheck om("test");
// }

TEST_F(VddkDeadlockCheckTest, GetInstance)
{
    StubClogToVoidLogNullPointReference();
    VddkDeadlockCheck om;
    om.GetInstance();
}

TEST_F(VddkDeadlockCheckTest, GenerateID)
{
    uint64_t requestID = 12345;
    std::string strApiName = "test";
    StubClogToVoidLogNullPointReference();

    VddkDeadlockCheck om;
    stub.set(ADDR(VddkDeadlockCheck, StartDeadlockThread), StubStartDeadlockThread);
    stub.set(ADDR(VddkDeadlockCheck, GenerateIDInner), StubGenerateIDInner);
    om.GenerateID(requestID, strApiName);

    stub.reset(ADDR(VddkDeadlockCheck, StartDeadlockThread));
    stub.reset(ADDR(VddkDeadlockCheck, GenerateIDInner));
}

TEST_F(VddkDeadlockCheckTest, ReleaseID)
{
    StubClogToVoidLogNullPointReference();
    uint64_t requestID = 12345;
    st_vddkApiInfo tmp;
    tmp.requestID = 12345;
    tmp.strName = "test";
    VddkDeadlockCheck om;
    om.m_invokingApis[12345] = tmp;
    om.ReleaseID(requestID);
}

// TEST_F(VddkDeadlockCheckTest, StartDeadlockThread)
// {
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     bool iRet = false;
//     VddkDeadlockCheck om;
//     stub.set(ADDR(MessageLoopThread, IsRunning), StubIsRunning);
//     stub.set(ADDR(MessageLoopThread, Start), StubStart);
//     stub.set(ADDR(VddkDeadlockCheck, KillProcess), StubKillProcess);
//     iRet = om.StartDeadlockThread();
//     EXPECT_EQ(true, iRet);

//     stub.reset(ADDR(MessageLoopThread, IsRunning));
//     stub.reset(ADDR(MessageLoopThread, Start));
//     stub.reset(ADDR(VddkDeadlockCheck, KillProcess));
// }

TEST_F(VddkDeadlockCheckTest, GenerateIDInner)
{
    StubClogToVoidLogNullPointReference();
    bool iRet = false;
    uint64_t requestID = 12345;
    std::string strApiName = "test";
    uint64_t tempID;
    st_vddkApiInfo tmp;
    tmp.requestID = 12345;
    tmp.strName = "test";
    VddkDeadlockCheck om;
    om.m_invokingApis[12345] = tmp;
    iRet = om.GenerateIDInner(requestID, strApiName, tempID);
    EXPECT_EQ(true, iRet);
}

TEST_F(VddkDeadlockCheckTest, DoDeadlockCheck)
{
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    st_vddkApiInfo tmp;
    tmp.requestID = 12345;
    tmp.strName = "test";
    tmp.invokeTime = std::chrono::steady_clock::now();
    VddkDeadlockCheck om;
    om.m_invokingApis[12345] = tmp;
    stub.set(ADDR(VddkDeadlockCheck, KillProcess), StubKillProcess);
    om.DoDeadlockCheck();
    stub.reset(ADDR(VddkDeadlockCheck, KillProcess));
}