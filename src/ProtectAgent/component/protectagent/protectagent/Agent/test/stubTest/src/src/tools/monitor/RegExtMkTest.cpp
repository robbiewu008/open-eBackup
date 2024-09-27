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
#include "tools/agentcli/RegExtMkTest.h"
#include "common/Utils.h"
#include "securecom/CryptAlg.h"

static mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}


static mp_void StubCPasswordGetInput(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "Admin";
}

mp_void StubCPasswordGetInputEchoWithStar(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "Admin@123";
}

TEST_F(CRegExtMkTest, Handle)
{
    RegExtMk work;
    mp_int32 iRet;

    stub.set(DoSleep,StubDoSleep);
    stub.set(&PBKDF2Hash, stub_return_success);
    stub.set(ADDR(CPassword, GetInput),StubCPasswordGetInput);
    stub.set(ADDR(CPassword, GetInputEchoWithStar),StubCPasswordGetInputEchoWithStar);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegExtMk,VerifyUserPasswd),stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegExtMk,CheckPlainText),stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegExtMk,CheckMkLifeDays),stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegExtMk,DecryptStoredPasswd),stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(RegisterExternalMK,stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegExtMk,SignAllScriptsWithNewMK),stub_return_success);
    iRet = work.Handle();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CRegExtMkTest, EncryptPasswdWithExtMk)
{
    RegExtMk work;
    mp_int32 iRet;

    stub.set(DoSleep,StubDoSleep);
    stub.set(ADDR(CPassword, GetInput),StubCPasswordGetInput);
    stub.set(ADDR(CPassword, GetInputEchoWithStar),StubCPasswordGetInputEchoWithStar);
    iRet = work.EncryptPasswdWithExtMk();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CRegExtMkTest, RollbackCiphertext)
{
    RegExtMk work;
    mp_int32 iRet;

    stub.set(DoSleep,StubDoSleep);
    stub.set(ADDR(CPassword, GetInput),StubCPasswordGetInput);
    stub.set(ADDR(CPassword, GetInputEchoWithStar),StubCPasswordGetInputEchoWithStar);
    work.RollbackCiphertext();
}

TEST_F(CRegExtMkTest, SignAllScriptsWithNewMK)
{
    RegExtMk work;
    mp_int32 iRet;

    stub.set(DoSleep,StubDoSleep);
    stub.set(ADDR(CPassword, GetInput),StubCPasswordGetInput);
    stub.set(ADDR(CPassword, GetInputEchoWithStar),StubCPasswordGetInputEchoWithStar);
    iRet = work.SignAllScriptsWithNewMK();
    EXPECT_EQ(iRet, MP_FAILED);
}