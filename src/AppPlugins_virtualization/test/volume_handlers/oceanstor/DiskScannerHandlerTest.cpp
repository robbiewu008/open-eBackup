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
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <system/System.hpp>
#include <common/Macros.h>
#include <common/Constants.h>
#include <log/Log.h>
#include <volume_handlers/oceanstor/DiskScannerHandler.h>

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;

namespace HDT_TEST {
const std::string g_hostUuid = "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE";
class DiskScannerHandlerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    std::string m_volId = "123";
    std::string m_volWwn = "112233445566";
    std::string m_volPath;
};

void DiskScannerHandlerTest::SetUp(){}
void DiskScannerHandlerTest::TearDown(){}
void DiskScannerHandlerTest::SetUpTestCase() {}
void DiskScannerHandlerTest::TearDownTestCase() {}

// ------------------------------------------------ 检验参数 ----------------------------------------
const std::string g_session1 =
    "Rescanning session [sid: 1, target: iqn.2006-08.com.huawei:oceanprotect:2100c0bc9afee3ae::20002:192.168.97.156, "
    "portal: 192.168.97.156,3260]]";
const std::string g_session2 = "Rescanning session [sid: 44, target: "
                               "iqn.2012-10.com.huawei.dsware:fa168449642ec40b.vbs.131073,portal: 88.7.1.1,3260]";

const std::string g_volWwn = "0x6c0bc9a100fee3ae0d027cf800000019";
const std::string g_targetDiskPath = "/dev/sdk";
const std::string g_devDiskPath = "lrwxrwxrwx 1 root root 12 Jul 13 20:02 wwn-0x5607ecd5f0f1c002 -> ../../sdk\n";
const std::string g_volName1 = "snapshot-2cea01e3-491b-4366-a99c-62549e5ec130";
const std::string g_volName2 = "volume-2ceafa1e3-fd1b-4566-a78c-gf549e5ec640";
const std::string g_diskLetter1 = "sdj";
const std::string g_diskLetter2 = "sdk";
const std::string g_hostSN = "6d1af429-57c0-40ec-bd3e-3c6aa7e75769";

static int Stub_RunShell_HostUuuid_OK(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput.push_back(g_hostUuid);
    return SUCCESS;
}

static int Stub_RunShell_Scan_OK(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput.push_back("lrwxrwxrwx. 1 root root   9 Jul 26 00:00 scsi-3112233445566 -> ../../sda");
    return SUCCESS;
}

static int Stub_RunShell_Scan_Failed(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    return FAILED;
}

std::set<std::string> stub_diskLetters;
int32_t Stub_GetAllDiskLetters(std::set<std::string> &diskLetters)
{
    return SUCCESS;
}

std::string stub_diskWWn;
int32_t Stub_GetIscsiDiskWwid(const std::string &diskLetter, std::string &diskWwn)
{
    return FAILED;
}

std::string Stub_GetDiskPathForWWNInSysDir(const std::string &volWwn)
{
    std::string path = "/dev/sdk";
    return path;
}

std::vector<std::string> stub_cmdOut;
static int Stub_RunShell_Success(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput = stub_cmdOut;
    return SUCCESS;
}

static int Stub_RunShell_Failed(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    return FAILED;
}

int32_t Stub_GetDisk83Page_Ok(void* obj, const std::string& strDevice, std::string& strLunWWN, std::string& strLunID)
{
    strLunID = "123";
    strLunWWN = "112233445566";
    return SUCCESS;
}

int32_t Stub_GetDisk83Page_Failed(void* obj, const std::string& strDevice, std::string& strLunWWN, std::string& strLunID)
{
    strLunID = "11";
    strLunWWN = "22";
    return FAILED;
}
/*
 * 测试用例： 获取主机机器码
 * 前置条件： 无
 * CHECK点： 成功获取主机机器码
 */
TEST_F(DiskScannerHandlerTest, GetHostUuid)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuuid_OK);
    std::string hostUuid = DiskScannerHandler::GetInstance()->GetHostUuid();
    EXPECT_EQ(hostUuid, g_hostUuid);
}

/**
 * 测试用例： 获得主机HOST SN
 * 前置条件： 内置部署无法拿到system的uuid，用SN替代
 * CHECK点： 成功
 */
TEST_F(DiskScannerHandlerTest, GetHostSNSuccess){
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    stub_cmdOut.push_back(g_hostSN);

    std::string hostUuid;

    int32_t ret = DiskScannerHandler::GetInstance()->GetHostSN(hostUuid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 获得主机HOST SN
 * 前置条件： 内置部署无法拿到system的uuid，用SN替代
 * CHECK点： 失败
 */
TEST_F(DiskScannerHandlerTest, GetHostSNFailed){
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);
    stub_cmdOut.push_back(g_hostSN);

    std::string hostUuid;

    int32_t ret = DiskScannerHandler::GetInstance()->GetHostSN(hostUuid);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得所有盘符
 * 前置条件： 内置，查询所有的盘符
 * CHECK点： 成功
 */
TEST_F(DiskScannerHandlerTest, GetAllDiskLettersSuccess)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    stub_cmdOut.clear();
    stub_cmdOut.push_back("sdk");
    std::set<std::string> diskLetters;

    int32_t ret = DiskScannerHandler::GetInstance()->GetAllDiskLetters(diskLetters);
    EXPECT_EQ(ret, SUCCESS);
};

/**
 * 测试用例： 获得所有盘符
 * 前置条件： 内置，查询所有的盘符
 * CHECK点： 失败
 */
TEST_F(DiskScannerHandlerTest, GetAllDiskLettersFailed)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);

    std::set<std::string> diskLetters;

    int32_t ret = DiskScannerHandler::GetInstance()->GetAllDiskLetters(diskLetters);
    EXPECT_EQ(ret, FAILED);
};

/**
 * 测试用例： 通过wwn查找到路径
 * 前置条件： 内置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 获得路径失败
 */
TEST_F(DiskScannerHandlerTest, GetDiskPathForWwnInSysDirFailed){
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success); 
    stub.set(ADDR(DiskScannerHandler, GetAllDiskLetters), Stub_GetAllDiskLetters);
    stub.set(ADDR(DiskScannerHandler, GetIscsiDiskWwid), Stub_GetIscsiDiskWwid);
    
    const std::string volWwn = g_volWwn;
    std::string ret = DiskScannerHandler::GetInstance()->GetDiskPathForWwnInSysDir(volWwn);
    
    EXPECT_EQ(ret, "");
}

/**
 * 测试用例： 通过wwn查找到路径
 * 前置条件： 内置，映射后通过WWN获得对应磁盘路径
 * CHECK点： 成功
 */
TEST_F(DiskScannerHandlerTest, GetIscsiDiskWwidSuccess)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    stub_cmdOut.clear();
    stub_cmdOut.push_back("naa.efakfja341kfdfajdkafjk");

    const std::string &diskLetter = "sdk";
    std::string diskWwn;
    const std::string matchWwn = "efakfja341kfdfajdkafjk";
    int32_t ret = DiskScannerHandler::GetInstance()->GetIscsiDiskWwid(diskLetter, diskWwn);

    EXPECT_EQ(ret, SUCCESS);
};

/*
 * 测试用例： 扫描获取盘符路径
 * 前置条件： 主机上已存在该盘
 * CHECK点： 成功获取磁盘路径
 */
TEST_F(DiskScannerHandlerTest, DoScanAfterAttachWhenDevExist)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Scan_OK);
    stub.set(ADDR(DiskScannerHandler, GetDisk83Page), Stub_GetDisk83Page_Ok);
    int32_t iRet = DiskScannerHandler::GetInstance()->DoScanAfterAttach(m_volId, m_volWwn, m_volPath);
    EXPECT_EQ(m_volPath, "/dev/disk/by-id/../../sda");
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 扫描获取盘符路径
 * 前置条件： 主机上不存在该盘
 * CHECK点： 获取磁盘路径失败
 */
TEST_F(DiskScannerHandlerTest, DoScanAfterAttachWhenDevNotExist)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Scan_Failed);
    stub.set(ADDR(DiskScannerHandler, GetDisk83Page), Stub_GetDisk83Page_Ok);
    int32_t iRet = DiskScannerHandler::GetInstance()->DoScanAfterAttach(m_volId, m_volWwn, m_volPath);
    EXPECT_EQ(iRet, FAILED);
}

/*
 * 测试用例： 删除盘符路径
 * 前置条件： 主机上不存在该盘
 * CHECK点： 删除磁盘路径成功
 */
TEST_F(DiskScannerHandlerTest, DetachVolumeWhenDevNotExist)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuuid_OK);
    stub.set(ADDR(DiskScannerHandler, GetDisk83Page), Stub_GetDisk83Page_Failed);
    int32_t iRet = DiskScannerHandler::GetInstance()->DetachVolume(m_volId, m_volWwn);
    EXPECT_EQ(iRet, SUCCESS);
}

/*
 * 测试用例： 删除盘符路径
 * 前置条件： 主机上存在该盘
 * CHECK点： 删除磁盘路径成功
 */
TEST_F(DiskScannerHandlerTest, DetachVolumeWhenDevExist)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_HostUuuid_OK);
    stub.set(ADDR(DiskScannerHandler, GetDisk83Page), Stub_GetDisk83Page_Ok);
    int32_t iRet = DiskScannerHandler::GetInstance()->DetachVolume(m_volId, m_volWwn);
    EXPECT_EQ(iRet, SUCCESS);
}
}
