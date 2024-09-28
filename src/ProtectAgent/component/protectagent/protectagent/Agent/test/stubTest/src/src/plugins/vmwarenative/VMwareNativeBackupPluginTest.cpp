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
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <unistd.h>
#include "gtest/gtest.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "stub.h"
#include "securec.h"
#include "plugins/vmwarenative/VMwareNativeBackupPlugin.h"

class VMwareNativeBackupPluginTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    Stub m_stub;
};

void VmfsHandlerTest::SetUp()
{}

void VmfsHandlerTest::TearDown()
{}

void VmfsHandlerTest::SetUpTestCase()
{}

void VmfsHandlerTest::TearDownTestCase()
{}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

/*
* ��������������VmnativeVmfsCheckTool
* ǰ��������VmfsCheckTool����ʧ��
* CHECK�㣺VmnativeVmfsCheckTool����ʧ��
* ǰ��������VmfsCheckTool���óɹ�
* CHECK�㣺VmnativeVmfsCheckTool���óɹ�
*/
TEST_F(VMwareNativeBackupPluginTest, VmnativeVmfsCheckTool)
{
    int iRet = MP_FAILED;
    VMwareNativeBackupPlugin VNBPObj;

    stub.set(ADDR(VMwareNativeBackup, VmfsCheckTool), StubFailed);
    iRet = VNBPObj.VmnativeVmfsCheckTool();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VMwareNativeBackup, VmfsCheckTool));
    stub.set(ADDR(VMwareNativeBackup, VmfsCheckTool), StubSuccess);
    iRet = VNBPObj.VmnativeVmfsCheckTool();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* ��������������VmnativeVmfsMount
* ǰ��������VmfsMount����ʧ��
* CHECK�㣺VmnativeVmfsMount����ʧ��
* ǰ��������VmfsMount���óɹ�
* CHECK�㣺VmnativeVmfsMount���óɹ�
*/
TEST_F(VMwareNativeBackupPluginTest, VmnativeVmfsMount)
{
    int iRet = MP_FAILED;
    VMwareNativeBackupPlugin VNBPObj;

    stub.set(ADDR(VMwareNativeBackup, VmfsMount), StubFailed);
    iRet = VNBPObj.VmnativeVmfsMount();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VMwareNativeBackup, VmfsMount));
    stub.set(ADDR(VMwareNativeBackup, VmfsMount), StubSuccess);
    iRet = VNBPObj.VmnativeVmfsMount();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* ��������������VmnativeVmfsUmount
* ǰ��������VmfsVmfsUmount����ʧ��
* CHECK�㣺VmnativeVmfsUmount����ʧ��
* ǰ��������VmfsUmount���óɹ�
* CHECK�㣺VmnativeVmfsUmount���óɹ�
*/
TEST_F(VMwareNativeBackupPluginTest, VmnativeVmfsUmount)
{
    int iRet = MP_FAILED;
    VMwareNativeBackupPlugin VNBPObj;

    stub.set(ADDR(VMwareNativeBackup, VmfsUmount), StubFailed);
    iRet = VNBPObj.VmnativeVmfsUmount();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VMwareNativeBackup, VmfsUmount));
    stub.set(ADDR(VMwareNativeBackup, VmfsUmount), StubSuccess);
    iRet = VNBPObj.VmnativeVmfsUmount();
    EXPECT_EQ(iRet, MP_SUCCESS);
}
