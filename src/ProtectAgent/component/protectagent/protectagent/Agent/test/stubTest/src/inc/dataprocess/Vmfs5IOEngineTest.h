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
#ifndef __VMFS_IO_ENGINE_TEST_H__
#define __VMFS_IO_ENGINE_TEST_H__

#define private public

#include "dataprocess/ioscheduler/Vmfs5IOEngine.h"
#include "common/ConfigXmlParse.h"
#include "common/File.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"


class vmfs5IOEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    Stub stub;
};

void vmfs5IOEngineTest::SetUp()
{}

void vmfs5IOEngineTest::TearDown()
{}

void vmfs5IOEngineTest::SetUpTestCase()
{}

void vmfs5IOEngineTest::TearDownTestCase()
{}

VmfsFsT StubReturnVmfsFsT()
{
    VmfsFsT* vft;
    vft->debugLevel = 0;
    vft->fs_info = NULL;
    vft->dev = NULL;
    return vft;
}

VmfsDirT StubReturnVmfsDirT()
{
    VmfsDirT* vdt;
    vdt->pos = 0;
    vdt->dir = NULL;
    vdt->dirent = NULL;
    return vdt;
}

VmfsDirentT StubReturnVmfsDirentT()
{
    VmfsDirentT* vdt;
    vdt->type = 0;
    vdt->blockId = 0;
    vdt->recordId = 0;
    return vdt;
}

mp_int32 StubSetDiskPathFail()
{
    return MP_FAILED;
}

mp_int32 StubSetDiskPathSucc()
{
    return MP_SUCCESS;
}
mp_int32 StubVmfsIOLookupFail()
{
    return MP_FAILED;
}

mp_int32 StubVmfsIOLookupSucc()
{
    return MP_SUCCESS;
}

#endif