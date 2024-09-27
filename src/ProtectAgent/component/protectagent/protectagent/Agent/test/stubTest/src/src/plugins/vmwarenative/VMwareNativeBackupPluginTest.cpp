/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
* 测试用例：调用VmnativeVmfsCheckTool
* 前置条件：VmfsCheckTool调用失败
* CHECK点：VmnativeVmfsCheckTool调用失败
* 前置条件：VmfsCheckTool调用成功
* CHECK点：VmnativeVmfsCheckTool调用成功
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
* 测试用例：调用VmnativeVmfsMount
* 前置条件：VmfsMount调用失败
* CHECK点：VmnativeVmfsMount调用失败
* 前置条件：VmfsMount调用成功
* CHECK点：VmnativeVmfsMount调用成功
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
* 测试用例：调用VmnativeVmfsUmount
* 前置条件：VmfsVmfsUmount调用失败
* CHECK点：VmnativeVmfsUmount调用失败
* 前置条件：VmfsUmount调用成功
* CHECK点：VmnativeVmfsUmount调用成功
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
