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
#include "apps/dws/DWSPluginTest.h"

#include <vector>
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Uuid.h"
#include "common/ConfigXmlParse.h"
#include "apps/dws/XBSAServer/ThriftServer.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/CTimer.h"
#include "common/Ip.h"
#include "securecom/Ip.h"

namespace {

mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stubDwsPlugin.set(ADDR(CLogger, Log), LogTest); \
} while (0)

static int g_iCountQueryPid = 0;
static int g_iCountQueryTime = 0;

mp_int32 StubInitReturnFailed()
{
    return MP_FAILED;
}

mp_int32 StubInitReturnSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubCreateReturnFailed()
{
    return MP_FAILED;
}

mp_int32 StubCreateReturnSuccess()
{
    return MP_SUCCESS;
}

mp_int32 Stub_GetValueInt32_Succ(mp_void* pthis, const mp_string& strSection, const mp_string& strKey,
    mp_int32& iValue)
{
    iValue = 100;
    return MP_SUCCESS;
}

mp_int32 Stub_GetTaskCacheInfo(const mp_string &taskId, DwsCacheInfo &cacheInfo)
{
    return MP_SUCCESS;
}

}

void DWSPluginTest::SetUp()
{
    ptrDwsPlugin = std::make_shared<DWSPlugin>();
}

TEST_F(DWSPluginTest, Init)
{
    DoGetJsonStringTest();
    std::vector<mp_uint32> cmds;
    mp_int32 iRet = MP_SUCCESS;

    stubDwsPlugin.set((mp_int32(ThriftServer::*)())ADDR(ThriftServer, Init), StubInitReturnFailed);
    iRet = ptrDwsPlugin->Init(cmds);
    EXPECT_EQ(MP_FAILED, iRet);

    stubDwsPlugin.set((mp_int32(ThriftServer::*)())ADDR(ThriftServer, Init), StubInitReturnSuccess);
    stubDwsPlugin.set((mp_int32(CTimer::*)())ADDR(CTimer, Init), StubInitReturnFailed);
    iRet = ptrDwsPlugin->Init(cmds);
    EXPECT_EQ(MP_FAILED, iRet);

    stubDwsPlugin.set((mp_int32(CTimer::*)())ADDR(CTimer, Init), StubInitReturnSuccess);
    stubDwsPlugin.set(ADDR(CMpThread, Create), StubCreateReturnFailed);
    iRet = ptrDwsPlugin->Init(cmds);
    EXPECT_EQ(MP_FAILED, iRet);

    stubDwsPlugin.set(ADDR(CMpThread, Create), StubCreateReturnSuccess);
    iRet = ptrDwsPlugin->Init(cmds);
    EXPECT_EQ(MP_SUCCESS, iRet);
}
