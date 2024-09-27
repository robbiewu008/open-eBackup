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
#include "common/SemafTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubSemReturnFailed(mp_semaf* pSemaf)
{
    return MP_FAILED;
}
mp_int32 StubSemReturnSuccess(mp_semaf* pSemaf)
{
    return MP_SUCCESS;
}

TEST_F(SemafTest, Init)
{
    CSemaf work;
    mp_int32 iRet;
    mp_semaf semaf;
    iRet = work.Init(semaf);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(SemafTest, Release)
{
    CSemaf work;
    mp_int32 iRet;
    mp_semaf semaf;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(sem_post, StubSemReturnFailed);
    iRet = work.Release(&semaf);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(sem_post, StubSemReturnSuccess);
    iRet = work.Release(&semaf);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(SemafTest, Wait)
{
    CSemaf work;
    mp_int32 iRet;
    mp_semaf semaf;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = work.Wait(&semaf);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(sem_wait, StubSemReturnFailed);
    iRet = work.Wait(&semaf);
    EXPECT_EQ(iRet, MP_FAILED);
}