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
#ifndef __FILE_IO_ENGINE_TEST_H__
#define __FILE_IO_ENGINE_TEST_H__

#define private public

#include "dataprocess/ioscheduler/FileIOEngine.h"
#include "common/ConfigXmlParse.h"
#include "common/File.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

class FileIOEngineTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void FileIOEngineTest::SetUp() {}

void FileIOEngineTest::TearDown() {}

void FileIOEngineTest::SetUpTestCase() {}

void FileIOEngineTest::TearDownTestCase() {}

mp_int32 StubFileIOEngineGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_bool StubFileExistTrue(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool StubFileExistFalse(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 StubCreateDirFail(const mp_char* pszDirPath)
{
    return MP_FAILED;
}

mp_int32 StubCreateDirSucc(const mp_char* pszDirPath)
{
    return MP_SUCCESS;
}

mp_int32 StubRetryOp(std::function<mp_int32()> internalOp) {
    return MP_SUCCESS;
}

#endif