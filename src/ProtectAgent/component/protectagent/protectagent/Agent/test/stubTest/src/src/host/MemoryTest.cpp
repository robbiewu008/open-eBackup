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
#include "host/MemoryTest.h"

static mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)
mp_void StubCLoggerLog(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFormat, ...)
{

}

static mp_int32 StubExecSystemWithEcho0(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect){
    
    mp_string memInfo1 = "MemTotal: 3898624 kB";
    mp_string memInfo2 = "MemFree: 584972 kB";
    mp_string memInfo3 = "SwapTotal: 4194300 kB";
    mp_string memInfo4 = "SwapFree: 2242108 kB";
    strEcho.push_back(memInfo1);
    strEcho.push_back(memInfo2);
    strEcho.push_back(memInfo3);
    strEcho.push_back(memInfo4);
    return 0;
}

TEST_F(MemoryTest, GetMemoryInfo)
{
    memory_info_t memInfo;
    mp_int32 iRet;
    CMemory work;
    stub.set(ADDR(CLogger,Log),StubCLoggerLog);
    stub.set(ADDR(CSystemExec,ExecSystemWithEcho), StubExecSystemWithEcho0);
    iRet = work.GetMemoryInfo(memInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
}