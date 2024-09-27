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
#ifndef __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_TEST_H__
#define __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_TEST_H__

#define private public
#include "dataprocess/vmwarenative/FileStorageDevice.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class FileStorageDeviceTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void FileStorageDeviceTest::SetUp() {}

void FileStorageDeviceTest::TearDown() {}

void FileStorageDeviceTest::SetUpTestCase() {}

void FileStorageDeviceTest::TearDownTestCase() {}

mp_uint64 StubGetNumberOfDataBlockCompletedSucc()
{
    return 0;
}

mp_bool StubPrepareForDiskCopy(const vmware_volume_info &volumeInfo, mp_string &folder, FILE **file,
    mp_uint64 backupLevel)
{
    return true;
}

mp_bool StubDeQueueToDataLunSucc(FILE *file, int done, int &condition)
{
    return true;
}

mp_bool StubGenerateDiskDescFile(
    mp_uint64 backupLevel, const vmware_volume_info& volumeInfo, const mp_string& folder)
{
    return true;
}

#endif