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
#include "plugins/DataProcessClientHandlerTest.h"
#include "plugins/DataProcessClientHandler.h"
#include "securecom/RootCaller.h"
namespace {
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub11.set(ADDR(CLogger, Log), LogTest); \
} while (0)

static mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

TEST_F(DataProcessClientHandlerTest, RemoveDpServiceClientTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataProcessClientHandler client;
    mp_string version;
    client.RemoveDpServiceClient(version);
}

TEST_F(DataProcessClientHandlerTest, GenerateDpServiceClientMapTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string dpParam;
    DataProcessClientHandler client;
    mp_int32 serviceType;
    mp_string pParam;
    client.GenerateDpServiceClientMap(serviceType, pParam);
}

TEST_F(DataProcessClientHandlerTest, FindDpClientTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    mp_string dpParam;
    DataProcessClientHandler client;
    mp_string version;
    client.FindDpClient(version);
}
