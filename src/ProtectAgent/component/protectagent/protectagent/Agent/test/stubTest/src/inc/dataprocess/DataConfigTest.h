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
#ifndef __DATACONFIGTEST_H__
#define __DATACONFIGTEST_H__
#define private public
#include "dataprocess/dataconfig/DataConfig.h"
#include "tinyxml2.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class CDataConfigTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void CDataConfigTest::SetUp() {}

void CDataConfigTest::TearDown() {}

void CDataConfigTest::SetUpTestCase() {}

void CDataConfigTest::TearDownTestCase() {}

mp_int32 StubGetValueInt32Success(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32Fail(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringSuccess(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringFail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubSetValueSuccess(const mp_string& strSection, const mp_string& strKey, mp_string strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSetValueFail(const mp_string& strSection, const mp_string& strKey, mp_string strValue)
{
    return MP_FAILED;
}

#endif
