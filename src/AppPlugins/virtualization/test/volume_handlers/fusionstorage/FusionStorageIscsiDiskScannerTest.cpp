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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <common/Constants.h>
#include "volume_handlers/oceanstor/DiskScannerHandler.h"
#include "volume_handlers/fusionstorage/FusionStorageIscsiDiskScanner.h"
#include "volume_handlers/fusionstorage/FusionStorageRestApiErrorCode.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ErrorCode = FusionStorageRestApiErrorCode;

using namespace VirtPlugin;

namespace HDT_TEST {
class FusionStorageIscsiDiskScannerTest : public testing::Test {

protected:
    void SetUp()
    {
        fusionStorageIscsiDiskScannerTest = FusionStorageIscsiDiskScanner::GetInstance();
    }
    void TearDown()
    {}

public:
    FusionStorageIscsiDiskScanner *fusionStorageIscsiDiskScannerTest;
};

// ------------------------------------------------ 检验参数 ----------------------------------------
const std::string g_session1 =
    "Rescanning session [sid: 1, target: iqn.2006-08.com.huawei:oceanprotect:2100c0bc9afee3ae::20002:192.168.97.156, "
    "portal: 192.168.97.156,3260]]";
const std::string g_session2 = "Rescanning session [sid: 44, target: "
                               "iqn.2012-10.com.huawei.dsware:fa168449642ec40b.vbs.131073,portal: 88.7.1.1,3260]";

const std::string g_loginedIp1 = "192.168.1.186";
const std::string g_loginedIp2 = "88.7.1.1";
const std::string g_volWwn = "0x6c0bc9a100fee3ae0d027cf800000019";
const std::string g_targetDiskPath = "/dev/sdk";
const std::string g_devDiskPath = "lrwxrwxrwx 1 root root 12 Jul 13 20:02 wwn-0x5607ecd5f0f1c002 -> ../../sdk\n";
const std::string g_volName1 = "snapshot-2cea01e3-491b-4366-a99c-62549e5ec130";
const std::string g_volName2 = "volume-2ceafa1e3-fd1b-4566-a78c-gf549e5ec640";
const std::string g_diskLetter1 = "sdj";
const std::string g_diskLetter2 = "sdk";
const std::string g_hostSN = "6d1af429-57c0-40ec-bd3e-3c6aa7e75769";

std::string Stub_GetNullDiskPath(const std::string &volWwn)
{
    std::string ans = "";
    return ans;
}

std::string diskPathGetByWWN = "/dev/sdk";
std::string Stub_GetDiskPathForWWN(const std::string &volWwn)
{
    return diskPathGetByWWN;
}

std::set<std::string> stub_diskPathSet;
int32_t Stub_GetIscsiDiskPathSet(std::set<std::string> &diskPathSet)
{
    diskPathSet = stub_diskPathSet;
    return SUCCESS;
}

bool g_isExist = false;
int32_t Stub_DoDeleteDiskFromPathSetSuccess(
    std::set<std::string> &diskPathSet, const std::string &targetDiskName, bool isExist)
{
    isExist = g_isExist;
    return SUCCESS;
}

int32_t Stub_DoDeleteDiskFromPathSetFailed(
    std::set<std::string> &diskPathSet, const std::string &targetDiskName, bool isExist)
{
    isExist = g_isExist;
    return FAILED;
}

void Stub_RescanDevice()
{}

static int32_t ExecStubSuccess()
{
    return SUCCESS;
}

static int32_t ExecStubFailed()
{
    return FAILED;
}

std::vector<std::string> stub_cmdOutInFusionStorage;
static int Stub_RunShell_Success(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput = stub_cmdOutInFusionStorage;
    return SUCCESS;
}

static int Stub_RunShell_Failed(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    return FAILED;
}

std::string Stub_GetAgent_Path()
{
    return "path";
}

static int Stub_RunCommand_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& result,
    const Module::CmdRunUser& cmdUser)
{
    result = stub_cmdOutInFusionStorage;
    return SUCCESS;
}

static int Stub_RunCommand_WithWhiteList_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& result,
    const std::unordered_set<std::string>& pathWhitelist, const Module::CmdRunUser& cmdUser)
{
    result = stub_cmdOutInFusionStorage;
    return SUCCESS;
}

/**
 * 测试用例： 获得已登录的target
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetLoginedTargetIPSuccess)
{
    Stub stub;
    stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Success);
    stub_cmdOutInFusionStorage.push_back(g_loginedIp1);

    std::vector<std::string> loginedTestIps;

    int32_t ret = fusionStorageIscsiDiskScannerTest->GetLoginedTargetIP(loginedTestIps);
    EXPECT_EQ(ret, SUCCESS);
};

/**
 * 测试用例： 获得已登录的target
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败于没有已登录的
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetLoginedTargetIPFailed1)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    std::vector<std::string> loginedTestIps;
    stub_cmdOutInFusionStorage = loginedTestIps;

    int32_t ret = fusionStorageIscsiDiskScannerTest->GetLoginedTargetIP(loginedTestIps);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 获得已登录的target
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败于运行CMD
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetLoginedTargetIPFailed2)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);
    std::vector<std::string> loginedTestIps;
    stub_cmdOutInFusionStorage = loginedTestIps;

    int32_t ret = fusionStorageIscsiDiskScannerTest->GetLoginedTargetIP(loginedTestIps);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 获得磁盘设备路径
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoScanAfterMappedSuccess0)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetDiskPathForWWN), Stub_GetDiskPathForWWN);

    const std::string volName = "volName1";
    const std::string volWwn = "volWwn1";
    std::string diskDevicePath;
    diskPathGetByWWN = "/temp23/123";

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoScanAfterMapped(volName, volWwn, diskDevicePath, false);
    EXPECT_EQ(ret, SUCCESS);
};

/**
 * 测试用例： 获得磁盘设备路径
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoScanAfterMappedFailed0)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetDiskPathForWWN), Stub_GetNullDiskPath);

    const std::string volName = "volName1";
    const std::string volWwn = "volWwn1";
    std::string diskDevicePath = "";

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoScanAfterMapped(volName, volWwn, diskDevicePath, false);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 获得磁盘设备路径
 * 前置条件： 内置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoScanAfterMappedSuccess1)
{
    Stub stub;
    stub.set(ADDR(DiskScannerHandler, GetDiskPathForWwnInSysDir), Stub_GetDiskPathForWWN);

    const std::string volName = "volName1";
    const std::string volWwn = "volWwn1";
    std::string diskDevicePath;
    diskPathGetByWWN = "/temp23/123";

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoScanAfterMapped(volName, volWwn, diskDevicePath, true);
    EXPECT_EQ(ret, SUCCESS);
};

/**
 * 测试用例： 获得磁盘设备路径
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoScanAfterMappedFailed1)
{
    Stub stub;
    stub.set(ADDR(DiskScannerHandler, GetDiskPathForWwnInSysDir), Stub_GetNullDiskPath);

    const std::string volName = "volName1";
    const std::string volWwn = "volWwn1";
    std::string diskDevicePath;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoScanAfterMapped(volName, volWwn, diskDevicePath, true);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 通过wwn查找到路径
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetDiskPathForWWNSuccess)
{
    Stub stub;
    stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Success);
    stub_cmdOutInFusionStorage.clear();
    stub_cmdOutInFusionStorage.push_back(g_devDiskPath);

    const std::string volWwn = "volWwn1";
    std::string ret = fusionStorageIscsiDiskScannerTest->GetDiskPathForWWN(volWwn);
    std::string path = "/dev/disk/by-id/../../sdk";

    EXPECT_EQ(ret, path);
};

/**
 * 测试用例： 通过wwn查找到路径
 * 前置条件： 外置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetDiskPathForWWNFailed)
{
    Stub stub;
    stub.set(ADDR(Module::EnvVarManager, GetAgentHomePath), Stub_GetAgent_Path);
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);
    const std::string volWwn = "volWwn1";
    std::string ret = fusionStorageIscsiDiskScannerTest->GetDiskPathForWWN(volWwn);
    std::string path = "";

    EXPECT_EQ(ret, path);
};

/**
 * 测试用例： 登出target
 * 前置条件： 结束IO操作后，需要登出target
 * CHECK点： 失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoLogOutFailed)
{
    const std::string targetIp = "192.199.6.6";
    int32_t ret = fusionStorageIscsiDiskScannerTest->DoLogOut(targetIp);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 删除磁盘集合
 * 前置条件： 映射后通过WWN获得对应磁盘路径
 * CHECK点： 删除成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DeleteDiskFromPathSetSuccess)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetIscsiDiskPathSet), ExecStubSuccess);
    stub.set(ADDR(FusionStorageIscsiDiskScanner, DoDeleteDiskFromPathSet), Stub_DoDeleteDiskFromPathSetSuccess);

    g_isExist = false;
    const std::string targetDiskPath = g_targetDiskPath;
    const std::string volName = g_volName1;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DeleteDiskFromPathSet(volName, targetDiskPath);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 删除磁盘集合
 * 前置条件： 映射后通过WWN获得对应磁盘路径
 * CHECK点： 删除失败于去除前缀
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DeleteDiskFromPathSetFailed1)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetIscsiDiskPathSet), ExecStubSuccess);
    stub.set(ADDR(FusionStorageIscsiDiskScanner, DoDeleteDiskFromPathSet), Stub_DoDeleteDiskFromPathSetFailed);

    g_isExist = true;
    const std::string targetDiskPath = "ddd";
    const std::string volName = g_volName1;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DeleteDiskFromPathSet(volName, targetDiskPath);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 删除磁盘集合
 * 前置条件： 映射后通过WWN获得对应磁盘路径
 * CHECK点： 删除失败于扫描路径集合
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DeleteDiskFromPathSetFailed2)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetIscsiDiskPathSet), ExecStubFailed);
    stub.set(ADDR(FusionStorageIscsiDiskScanner, DoDeleteDiskFromPathSet), ExecStubSuccess);

    const std::string targetDiskPath = g_targetDiskPath;
    const std::string volName = g_volName1;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DeleteDiskFromPathSet(volName, targetDiskPath);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得映射磁盘集合
 * 前置条件： 删除磁盘前获得磁盘集合
 * CHECK点： 获得成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetIscsiDiskPathSetSuccess)
{
    Stub stub;
    stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Success);

    stub_cmdOutInFusionStorage.push_back(g_targetDiskPath);
    std::set<std::string> diskPathSet;

    int32_t ret = fusionStorageIscsiDiskScannerTest->GetIscsiDiskPathSet(diskPathSet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 获得映射磁盘集合
 * 前置条件： 删除磁盘前获得磁盘集合
 * CHECK点： 获得失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, GetIscsiDiskPathSetFailed)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);

    std::set<std::string> diskPathSet;

    int32_t ret = fusionStorageIscsiDiskScannerTest->GetIscsiDiskPathSet(diskPathSet);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 执行删除磁盘操作
 * 前置条件： 获得磁盘集合
 * CHECK点： 执行删除行为成功
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoDeleteDiskFromPathSetSuccess)
{
    Stub stub;
    stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const std::unordered_set<std::string>&, const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_WithWhiteList_Success);

    std::set<std::string> diskPathSet;
    diskPathSet.insert(g_targetDiskPath);
    std::string targetDiskName = g_diskLetter2;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoDeleteDiskFromPathSet(diskPathSet, targetDiskName, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 执行删除磁盘操作
 * 前置条件： 获得映射磁盘集合
 * CHECK点： 执行删除失败
 */
TEST_F(FusionStorageIscsiDiskScannerTest, DoDeleteDiskFromPathSetFailed)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);

    std::set<std::string> diskPathSet;
    diskPathSet.insert(g_targetDiskPath);
    std::string targetDiskName = g_diskLetter2;

    int32_t ret = fusionStorageIscsiDiskScannerTest->DoDeleteDiskFromPathSet(diskPathSet, targetDiskName, false);
    EXPECT_EQ(ret, FAILED);
}

}  // namespace HDT_TEST
