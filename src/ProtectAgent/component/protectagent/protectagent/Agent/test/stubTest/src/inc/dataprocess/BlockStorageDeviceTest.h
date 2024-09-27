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
#ifndef __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_TEST_H__
#define __AGENT_VMWARENATIVE_BLOCKSTORAGEDEVICE_TEST_H__

#define private public
#include "dataprocess/vmwarenative/BlockStorageDevice.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"
class BlockStorageDeviceTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void BlockStorageDeviceTest::SetUp() {}

void BlockStorageDeviceTest::TearDown() {}

void BlockStorageDeviceTest::SetUpTestCase() {}

void BlockStorageDeviceTest::TearDownTestCase() {}

mp_int32 StubFileBlockStorageDeviceGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

bool StubDeQueueToDataLun(FILE *file, int done, int &condition)
{
    return true;
}

mp_uint64 StubGetNumberOfDataBlockCompleted()
{
    return 0;
}

#endif