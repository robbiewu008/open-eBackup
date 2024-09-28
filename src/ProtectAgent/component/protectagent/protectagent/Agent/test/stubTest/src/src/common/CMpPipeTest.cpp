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
#include "common/CMpPipeTest.h"

namespace {
mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}
TEST_F(CMpPipeTest, ReadPipeTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    std::vector<mp_string> vecOutput;
    mp_string strFileName("/bin/test");
    CMpPipe cmpPipe;
    cmpPipe.ReadPipe(strFileName, vecOutput);
}

TEST_F(CMpPipeTest, WritePipeTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strFileName("/bin/test");
    std::vector<mp_string> vecInput;
    CMpPipe cmpPipe;
    vecInput.push_back("aaaa");
    cmpPipe.WritePipe(strFileName, vecInput);
}

TEST_F(CMpPipeTest, WritePipeInnerTest)
{
    mp_string strFileName;
    mp_string strInput;
    int nByteWrite;
    CMpPipe cmpPipe;

    stub.set(&CLogger::Log, StubCLoggerLog);
    cmpPipe.WritePipeInner(strFileName, strInput, nByteWrite);
}

TEST_F(CMpPipeTest, SetVecInputTest)
{
    mp_string strFileName("/bin/test");
    std::vector<mp_string> vecInput;
    CMpPipe cmpPipe;
    vecInput.push_back("aaaa");
    cmpPipe.SetVecInput(vecInput);
}

TEST_F(CMpPipeTest, GetVecInputTest)
{
    mp_string strFileName("/bin/test");
    std::vector<mp_string> vecInput;
    CMpPipe cmpPipe;
    vecInput.push_back("aaaa");
    cmpPipe.GetVecInput(vecInput);
}

TEST_F(CMpPipeTest, ReadInputTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strFileName("/bin/test");
    std::vector<mp_string> vecInput;
    CMpPipe cmpPipe;
    vecInput.push_back("aaaa");
    mp_string strUniqueID;
    mp_string strInput;

    stub.set(&CMpPipe::ReadPipe, StubFailed);
    mp_int32 iRet = cmpPipe.ReadInput(strUniqueID, strInput);
    EXPECT_EQ(iRet, MP_FAILED);
}
