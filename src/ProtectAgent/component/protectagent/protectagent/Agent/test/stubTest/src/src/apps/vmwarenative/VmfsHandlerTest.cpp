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
#include "apps/vmwarenative/VmfsHandler.h"

class VmfsHandlerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
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

mp_int32 Stub_Exec_Success(void *obj, mp_int32 iCommandID, const mp_string& strParam,
    std::vector<mp_string> pvecResult[], mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 Stub_Exec_Fail(void *obj, mp_int32 iCommandID, const mp_string& strParam,
    std::vector<mp_string> pvecResult[], mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_FAILED;
}

/*
* 测试用例：检查VMFS工具状态
* 前置条件：调用检查脚本接口返回成功
* CHECK点：检查返回成功
* 前置条件：调用检查脚本接口返回失败
* CHECK点：检查返回失败
*/
TEST_F(VmfsHandlerTest, CheckTool)
{
    int iRet = MP_FAILED;
    VmfsHandler VHObj;
    Stub stub;

    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Success);
    iRet = VHObj.CheckTool();
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.reset(ADDR(CRootCaller, Exec));
    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Fail);
    iRet = VHObj.CheckTool();
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 测试用例：VMFS挂载
* 前置条件：入参为空列表
* CHECK点：挂载返回失败
* 前置条件：入参为非空列表
* 前置条件：调用挂载脚本接口返回失败
* CHECK点：挂载返回失败
* 前置条件：调用挂载脚本接口返回成功
* CHECK点：挂载返回成功
* CHECK点：出参返回正确的挂载路径
*/
TEST_F(VmfsHandlerTest, Mount)
{
    int iRet = MP_FAILED;
    VmfsHandler VHObj;
    Stub stub;
    mp_string wwnNum = "123321";
    std::vector<mp_string> wwn;
    mp_string mountpoint;

    iRet = VHObj.Mount(wwn, mountpoint);
    EXPECT_EQ(iRet, MP_FAILED);

    wwn.push_back(wwnNum);

    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Fail);
    iRet = VHObj.Mount(wwn, mountpoint);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(CRootCaller, Exec));
    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Success);
    iRet = VHObj.Mount(wwn, mountpoint);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(mountpoint, VMFS_MOUNT_PATH + wwn[0]);
}

/*
* 测试用例：VMFS解挂载
* 前置条件：入参为空字符串
* CHECK点：解挂载返回失败
* 前置条件：入参为非空字符串
* 前置条件：调用解挂载脚本接口返回失败
* CHECK点：解挂载返回失败
* 前置条件：调用解挂载脚本接口返回成功
* CHECK点：解挂载返回成功
*/
TEST_F(VmfsHandlerTest, Umount)
{
    int iRet = MP_FAILED;
    VmfsHandler VHObj;
    Stub stub;
    mp_string mountpoint;

    iRet = VHObj.Umount(mountpoint);
    EXPECT_EQ(iRet, MP_FAILED);

    mountpoint = "123321"

    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Fail);
    iRet = VHObj.Umount(mountpoint);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(CRootCaller, Exec));
    stub.set(ADDR(CRootCaller, Exec), Stub_Exec_Success);
    iRet = VHObj.Umount(mountpoint);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
