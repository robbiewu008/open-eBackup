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
#ifndef _PREPARE_FILE_SYSTEM_TEST_H
#define _PREPARE_FILE_SYSTEM_TEST_H

#include "stub.h"
#include "gtest/gtest.h"
#include "common/Types.h"

#define private public  // hack complier
#define protected public

class PrepareFileSystemTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void PrepareFileSystemTest::SetUp() {}

void PrepareFileSystemTest::TearDown() {}

void PrepareFileSystemTest::SetUpTestCase() {}

void PrepareFileSystemTest::TearDownTestCase() {}

mp_int32 StubCheckHostLinkStatusTestSuccess(const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckHostLinkStatusTestFail(const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_FAILED;
}

mp_int32 StubExecTestSuccess(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_SUCCESS;
}

mp_int32 StubExecTestFailed(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_FAILED;
}

mp_int32 StubExecTestFail(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_FAILED;
}

static mp_int32 execNum = 0;
mp_int32 StubExecTest1fstSuccessOtherFailed(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    if (execNum == 0) {
        execNum++;
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

mp_int32 StubCheckHostIpLinkStatusTestSuccess(const mp_string &storageIp)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckHostIpLinkStatusTestFail(const mp_string &storageIp)
{
    return MP_FAILED;
}

mp_int32 StubCheckMountHostLinkStatusTestSuccess(const mp_string& iterIp)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckMountHostLinkStatusTestFailed(const mp_string& iterIp)
{
    return MP_FAILED;
}


mp_int32 StubPrepareFileSystemGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubPrepareFileSystemGetValueInt32ReturnFail(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubPrepareFileSystemGetValueStringSuccess(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "mountparam";
    return MP_SUCCESS;
}

mp_int32 StubPrepareFileSystemGetValueStringFail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_bool StubDirExistFail(const mp_char* pszDirPath)
{
    return MP_FALSE;
}

mp_bool StubDirExistSuccess(const mp_char* pszDirPath)
{
    return MP_TRUE;
}

mp_int32 StubCreateDirSuccess(const mp_char* pszDirPath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateDirFail(const mp_char* pszDirPath)
{
    return MP_FAILED;
}

mp_int32 StubGetHostSN(mp_string &strSN)
{
    strSN = "test_HostSN";
    return MP_SUCCESS;
}

mp_int32 StubGetHostSNFailed(mp_string &strSN)
{
    return MP_FAILED;
}

#endif